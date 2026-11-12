#include "GuiModule.h"
#include "ILuauHost.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "FileDialog.h"
#include "GtkLoop.h"
#include "Nodes.h"
#include "MainThreadCall.h"

#include "lua.h"
#include "lualib.h"
#include <glib.h>
#include <memory>
#include <stdexcept>
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

void GuiModule::run() {
    // Returns at once without a shown window, which nothing would close to end the loop
    runGtkLoop();
    loopEnded = true;
}

// A call another thread posted, run by the main loop
static gboolean runMainThreadCall(gpointer data) {
    auto* call = static_cast<std::shared_ptr<MainThreadCall>*>(data);
    (*call)->run();
    delete call;
    return G_SOURCE_REMOVE;
}

bool GuiModule::runOnMainThread(const std::function<void()>& work) {
    if (isMainThread()) {
        work();
        return true;
    }
    if (loopEnded) return false;

    auto call = std::make_shared<MainThreadCall>(work);
    g_idle_add(runMainThreadCall, new std::shared_ptr<MainThreadCall>(call));
    Luwow::Engine::StateRelease release(host);
    return call->wait(loopEnded);
}

Luwow::Engine::RunMode GuiModule::getRunMode() const {
    return LUWOW_MODULE_RUN_MODE;
}

void GuiModule::setHost(ILuauHost* host) {
    this->host = host;
    mainThread = std::this_thread::get_id();
    ensureGtkInitialized();
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
