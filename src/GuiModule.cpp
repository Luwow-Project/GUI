#include "GuiModule.h"
#include "ControlNodes/Window.h"
// The platform's controls, from its folder
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "FileDialog.h"

#include "lua.h"
#include "lualib.h"
#include <stdexcept>

namespace Luwow::Gui {

// Ends the main loop, which each platform's Platform file implements
void quitMainLoop();

namespace {

// The windows shown and not yet closed
int shownWindows = 0;
// A quit before the loop runs would end it as soon as it starts, so the last window closing before then
// quits nothing
bool loopRunning = false;

GuiModule* getModuleInstance(lua_State* L) {
    GuiModule* gui = static_cast<GuiModule*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!gui) {
        throw std::runtime_error("Gui module not found");
    }
    return gui;
}

int createWindowNow(lua_State* L) {
    return ControlNode::Window::create(L, getModuleInstance(L));
}

// gui.showFileDialog(descriptor, parent?), the chosen paths, nil when it's cancelled
int showFileDialogNow(lua_State* L) {
    FileDialogDescriptor descriptor = getFileDialogDescriptor(L, 1);
    IWindow* parent = nullptr;
    if (!lua_isnoneornil(L, 2)) {
        auto* window = dynamic_cast<ControlNode::Window*>(nodeArg(L, 2).get());
        if (!window) luaL_typeerror(L, 2, ControlNode::Window::TypeName);
        parent = window->control.get();
    }
    std::vector<std::string> paths = getModuleInstance(L)->showFileDialog(descriptor, parent);
    if (paths.empty()) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, (int)paths.size(), 0);
    for (size_t i = 0; i < paths.size(); ++i) {
        lua_pushlstring(L, paths[i].data(), paths[i].size());
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

// Called from another thread, the functions run on the main thread while the caller waits
int createWindowExport(lua_State* L) {
    return callOnMainThread(L, createWindowNow);
}

int showFileDialogExport(lua_State* L) {
    return callOnMainThread(L, showFileDialogNow);
}

LuauExport exports[] = {
    { "createWindow", createWindowExport },
    { "showFileDialog", showFileDialogExport },
    { nullptr, nullptr }
};

} // namespace

void windowShown() {
    ++shownWindows;
}

void windowClosed() {
    if (--shownWindows == 0 && loopRunning) quitMainLoop();
}

Luwow::Engine::ILuauModule* GuiModule::initialize(Luwow::Engine::ILuauHost* host) {
    GuiModule* gui = new GuiModule();
    gui->host = host;
    gui->mainThread = std::this_thread::get_id();
    gui->startPlatform();
    registerNodes(host->getMainState(), host, gui);
    return gui;
}

void GuiModule::run() {
    if (shownWindows > 0) {
        loopRunning = true;
        runLoop();
        loopRunning = false;
    }
    loopEnded = true;
}

bool GuiModule::runOnMainThread(const std::function<void()>& work) {
    if (isMainThread()) {
        work();
        return true;
    }
    if (loopEnded) return false;

    auto call = std::make_shared<MainThreadCall>(work);
    if (!post(call)) return false;
    Luwow::Engine::StateRelease release(host);
    return call->wait(loopEnded);
}

Luwow::Engine::RunMode GuiModule::getRunMode() const {
    return LUWOW_MODULE_RUN_MODE;
}

const char* GuiModule::getModuleName() const {
    return "gui";
}

const char* GuiModule::getModuleAlias() const {
    return "Luwow";
}

const LuauExport* GuiModule::getExports() const {
    return exports;
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

} // namespace Luwow::Gui

LUWOW_REGISTER_MODULE(Luwow::Gui::GuiModule)
