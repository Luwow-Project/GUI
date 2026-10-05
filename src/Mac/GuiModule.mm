#import <Cocoa/Cocoa.h>

#include "GuiModule.h"
#include "ILuauHost.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Handles.h"

#include "lua.h"
#include "lualib.h"
#include <stdexcept>
#include <string>

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

// Gets the module for a Luau call, which must come from the main thread
static GuiModule* getMainThreadInstance(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    if (!gui->isMainThread()) {
        luaL_error(L, "gui can only be used from the main thread");
    }
    return gui;
}

GuiModule::GuiModule() : host(nullptr) {}

ILuauModule* GuiModule::initialize(ILuauHost* host) {
    GuiModule* gui = new GuiModule();
    gui->setHost(host);
    registerHandles(host->getMainState());
    return gui;
}


void GuiModule::run() {
    if (!createdWindow) return; // Without a window there's nothing to wait for, and the loop would never end
    @autoreleasepool {
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
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
    createdWindow = true;
    return new Window(descriptor);
}

IButton* GuiModule::createButton(const ButtonDescriptor& descriptor, IWindow* parent) {
    return new Button(descriptor, parent);
}

IMenuBar* GuiModule::createMenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) {
    return new MenuBar(descriptor, parent);
}

static int createWindow(lua_State* L) {
    GuiModule* gui = getMainThreadInstance(L);
    WindowDescriptor windowDescriptor = getWindowDescriptor(L);
    LuauWindow::Push(L, gui->createWindow(windowDescriptor));
    return 1;
}

static int createButton(lua_State* L) {
    GuiModule* gui = getMainThreadInstance(L);
    ButtonDescriptor buttonDescriptor = getButtonDescriptor(L);
    buttonDescriptor.host = gui->getHost();
    IWindow* parent = LuauWindow::Check(L, 2)->get();

    LuauButton::Push(L, gui->createButton(buttonDescriptor, parent));
    return 1;
}

static int createMenuBar(lua_State* L) {
    GuiModule* gui = getMainThreadInstance(L);
    MenuBarDescriptor menuBarDescriptor = getMenuBarDescriptor(L);
    for (MenuDescriptor& menu : menuBarDescriptor.Menus) {
        for (MenuItemDescriptor& item : menu.Items) item.host = gui->getHost();
    }
    IWindow* parent = LuauWindow::Check(L, 2)->get();

    LuauMenuBar::Push(L, gui->createMenuBar(menuBarDescriptor, parent));
    return 1;
}

const char* GuiModule::getModuleName() const {
    return "gui";
}

const char* GuiModule::getModuleAlias() const {
    return "Luwow";
}

static LuauExport exports[] = {
    { "createWindow", createWindow },
    { "createButton", createButton },
    { "createMenuBar", createMenuBar },
    { nullptr, nullptr }
};

const LuauExport* GuiModule::getExports() const {
    return exports;
}

} // namespace Luwow::Gui

LUWOW_REGISTER_MODULE(Luwow::Gui::GuiModule)
