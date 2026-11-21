#include "GuiModule.h"
#include "ControlNodes/Window.h"
// The platform's controls, from its folder
#include "Window.h"
#include "Button.h"
#include "TextLabel.h"
#include "TextBox.h"
#include "TextArea.h"
#include "Dropdown.h"
#include "Checkbox.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "FileDialog.h"

#include "lua.h"
#include "lualib.h"
#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

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

int createComponentNow(lua_State* L) {
    return Luwow::Gui::createComponent(L, getModuleInstance(L));
}

// gui.showFileDialog(descriptor, parent?), the chosen paths, nil when it's cancelled
int showFileDialogNow(lua_State* L) {
    FileDialogDescriptor descriptor = getFileDialogDescriptor(L, 1);
    IWindow* parent = nullptr;
    if (!lua_isnoneornil(L, 2)) {
        auto* window = dynamic_cast<ControlNode::Window*>(controlArg(L, 2).get());
        if (!window) luaL_typeerror(L, 2, ControlNode::Window::TypeName);
        parent = window->native.get();
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
int createComponentExport(lua_State* L) {
    return callOnMainThread(L, createComponentNow);
}

int showFileDialogExport(lua_State* L) {
    return callOnMainThread(L, showFileDialogNow);
}

// The services of IGuiModule.h's Topics
void nativeHandleRequest(void* context, Luwow::Engine::Message& message) {
    (void)context;
    std::shared_ptr<Control> control = controlArg(message.state, -1);
    if (!control) return;
    message.data = std::string(control->typeName()) + " " + std::to_string(reinterpret_cast<uintptr_t>(nativeHandle(*control)));
    message.result = 1;
}

void runOnMainThreadRequest(void* context, Luwow::Engine::Message& message) {
    uintptr_t function = 0;
    uintptr_t argument = 0;
    if (std::sscanf(message.data.c_str(), "%" SCNuPTR " %" SCNuPTR, &function, &argument) != 2 || !function) return;
    auto call = reinterpret_cast<void (*)(void*)>(function);
    bool ran = static_cast<GuiModule*>(context)->runOnMainThread([&] { call(reinterpret_cast<void*>(argument)); });
    message.result = ran ? 1 : 0;
}

void mainThreadWaitRequest(void* context, Luwow::Engine::Message& message) {
    auto* gui = static_cast<GuiModule*>(context);
    if (!gui->isMainThread()) return;
    int milliseconds = std::clamp(std::atoi(message.data.c_str()), 0, 1000);
    {
        Luwow::Engine::StateRelease release(gui->getHost());
        gui->runEvents(milliseconds);
    }
    message.result = 1;
}

LuauExport exports[] = {
    { "createComponent", createComponentExport },
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
    registerControls(host->getMainState(), host, gui);
    host->handle(Topics::NativeHandle, nativeHandleRequest, gui);
    host->handle(Topics::RunOnMainThread, runOnMainThreadRequest, gui);
    host->handle(Topics::MainThreadWait, mainThreadWaitRequest, gui);
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

ITextLabel* GuiModule::createTextLabel(const TextLabelDescriptor& descriptor, IWindow* window) {
    return new TextLabel(descriptor, window);
}

ITextBox* GuiModule::createTextBox(const TextBoxDescriptor& descriptor, IWindow* window) {
    return new TextBox(descriptor, window);
}

ITextArea* GuiModule::createTextArea(const TextAreaDescriptor& descriptor, IWindow* window) {
    return new TextArea(descriptor, window);
}

IDropdown* GuiModule::createDropdown(const DropdownDescriptor& descriptor, IWindow* window) {
    return new Dropdown(descriptor, window);
}

ICheckbox* GuiModule::createCheckbox(const CheckboxDescriptor& descriptor, IWindow* window) {
    return new Checkbox(descriptor, window);
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
