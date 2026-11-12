#import <Cocoa/Cocoa.h>

#include "GuiModule.h"
#include "ILuauHost.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "FileDialog.h"
#include "Nodes.h"
#include "MainThreadCall.h"

#include "lua.h"
#include "lualib.h"
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace Luwow::Gui {
using ILuauModule = Luwow::Engine::ILuauModule;
using ILuauHost = Luwow::Engine::ILuauHost;

static GuiModule* getModuleInstance(lua_State* L) {
    GuiModule* gui = static_cast<GuiModule*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!gui) {
        throw std::runtime_error("Gui module not found");
    }
    return gui;
}

GuiModule::GuiModule() : host(nullptr) {}

ILuauModule* GuiModule::initialize(ILuauHost* host) {
    GuiModule* gui = new GuiModule();
    gui->setHost(host);
    registerNodes(host->getMainState(), host, gui);
    return gui;
}


// The windows shown and not yet closed. A window that's never shown doesn't keep the loop running.
static int shownWindows = 0;
static bool loopRunning = false;

void windowShown() {
    ++shownWindows;
}

// The last shown window closing stops the run loop. stop: takes effect once an event is handled, so an
// empty one is posted for it.
void windowClosed() {
    if (--shownWindows > 0 || !loopRunning) return;
    [NSApp stop:nil];
    NSEvent* wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint modifierFlags:0
        timestamp:0 windowNumber:0 context:nil subtype:0 data1:0 data2:0];
    [NSApp postEvent:wake atStart:YES];
}

void GuiModule::run() {
    // Without a shown window there's nothing to wait for, and the loop would never end
    if (shownWindows == 0) {
        loopEnded = true;
        return;
    }
    loopRunning = true;
    @autoreleasepool {
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
    loopRunning = false;
    loopEnded = true;
}

bool GuiModule::runOnMainThread(const std::function<void()>& work) {
    if (isMainThread()) {
        work();
        return true;
    }
    if (loopEnded) return false;

    std::shared_ptr<MainThreadCall> call = std::make_shared<MainThreadCall>(work);
    dispatch_async(dispatch_get_main_queue(), ^{
        call->run();
    });
    Luwow::Engine::StateRelease release(host);
    return call->wait(loopEnded);
}

Luwow::Engine::RunMode GuiModule::getRunMode() const {
    return LUWOW_MODULE_RUN_MODE;
}

void GuiModule::setHost(ILuauHost* host) {
    this->host = host;
    mainThread = std::this_thread::get_id();

    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
    }
}

IWindow* GuiModule::createWindow(const WindowDescriptor& descriptor) {
    return new Window(descriptor);
}

IButton* GuiModule::createButton(const ButtonDescriptor& descriptor, IWindow* window) {
    return new Button(descriptor, window);
}

IMenuBar* GuiModule::createMenuBar(IWindow* window) {
    return new MenuBar(window);
}

IMenu* GuiModule::createMenu(const MenuDescriptor& descriptor) {
    return new Menu(descriptor);
}

IMenuItem* GuiModule::createMenuItem(const MenuItemDescriptor& descriptor, IWindow* window) {
    return new MenuItem(descriptor, window);
}

std::vector<std::string> GuiModule::showFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) {
    return showNativeFileDialog(descriptor, parent);
}

static int createWindowNow(lua_State* L) {
    return Luwow::Gui::createWindow(L, getModuleInstance(L));
}

static int showFileDialogNow(lua_State* L) {
    return Luwow::Gui::showFileDialog(L, getModuleInstance(L));
}

// Called from another thread, the functions run on the main thread while the caller waits
static int createWindow(lua_State* L) {
    return callOnMainThread(L, createWindowNow);
}

static int showFileDialog(lua_State* L) {
    return callOnMainThread(L, showFileDialogNow);
}

const char* GuiModule::getModuleName() const {
    return "gui";
}

const char* GuiModule::getModuleAlias() const {
    return "Luwow";
}

static LuauExport exports[] = {
    { "createWindow", createWindow },
    { "showFileDialog", showFileDialog },
    { nullptr, nullptr }
};

const LuauExport* GuiModule::getExports() const {
    return exports;
}

} // namespace Luwow::Gui

LUWOW_REGISTER_MODULE(Luwow::Gui::GuiModule)
