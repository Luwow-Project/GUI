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
#include <string>
#include <memory>
#include <vector>
#include <windows.h>

namespace Luwow::Gui {
using ILuauModule = Luwow::Engine::ILuauModule;
using ILuauHost = Luwow::Engine::ILuauHost;

// For all methods that require the Gui instance, we need to get it from the userdata.
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

// The main thread's hidden window, which other threads post their gui calls to
constexpr UINT WM_LUWOW_GUI_CALL = WM_APP + 0x4F;
static HWND dispatcherWindow = NULL;

static LRESULT CALLBACK DispatcherProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_LUWOW_GUI_CALL) {
        auto* call = reinterpret_cast<std::shared_ptr<MainThreadCall>*>(lParam);
        (*call)->run();
        delete call;
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// The windows shown and not yet closed. A window that's never shown doesn't keep the loop running.
static int shownWindows = 0;
// A quit posted before the loop runs would end it as soon as it starts, so the last window closing
// before then posts none
static bool loopRunning = false;

void windowShown() {
    ++shownWindows;
}

void windowClosed() {
    if (--shownWindows == 0 && loopRunning) PostQuitMessage(0);
}

static Window* GetWindow(HWND hWnd) {
    return (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
        {
            if (Window* window = GetWindow(hWnd)) window->destroyed();
            break;
        }
        case WM_MOVE:
        case WM_SIZE:
        {
            Window* window = GetWindow(hWnd);
            if (window && !IsIconic(hWnd)) window->boundsChanged();
            break;
        }
        case WM_COMMAND:
        {
            Window* window = GetWindow(hWnd);
            if (!window) break;
            // A control removed since the message was posted is no longer there
            uint16_t commandId = LOWORD(wParam);
            if (ICommandControl* commandControl = window->getCommandControl(commandId)) {
                commandControl->onCommand();
            }
            break;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


void GuiModule::run() {
    // Without a shown window there's nothing to wait for, and the loop would never end
    if (shownWindows == 0) {
        loopEnded = true;
        return;
    }
    loopRunning = true;
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    loopRunning = false;
    loopEnded = true;
}

bool GuiModule::runOnMainThread(const std::function<void()>& work) {
    if (isMainThread()) {
        work();
        return true;
    }
    if (!dispatcherWindow || loopEnded) return false;

    auto call = std::make_shared<MainThreadCall>(work);
    auto* message = new std::shared_ptr<MainThreadCall>(call);
    if (!PostMessage(dispatcherWindow, WM_LUWOW_GUI_CALL, 0, reinterpret_cast<LPARAM>(message))) {
        delete message;
        return false;
    }
    Luwow::Engine::StateRelease release(host);
    return call->wait(loopEnded);
}

Luwow::Engine::RunMode GuiModule::getRunMode() const {
    return LUWOW_MODULE_RUN_MODE;
}

void GuiModule::setHost(ILuauHost* host) {
    this->host = host;
    mainThread = std::this_thread::get_id();

    WNDCLASSEXA wcex;
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "LuwowWindow";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExA(&wcex);

    WNDCLASSEXA dispatcher = {};
    dispatcher.cbSize = sizeof(WNDCLASSEXA);
    dispatcher.lpfnWndProc = DispatcherProc;
    dispatcher.hInstance = GetModuleHandle(NULL);
    dispatcher.lpszClassName = "LuwowGuiDispatcher";
    RegisterClassExA(&dispatcher);
    dispatcherWindow = CreateWindowExA(0, "LuwowGuiDispatcher", "", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);
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
