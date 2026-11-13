#include "GuiModule.h"
#include "Window.h"

#include <windows.h>

namespace Luwow::Gui {

namespace {

// The main thread's hidden window, which other threads post their gui calls to
constexpr UINT WM_LUWOW_GUI_CALL = WM_APP + 0x4F;
HWND dispatcherWindow = NULL;

LRESULT CALLBACK DispatcherProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_LUWOW_GUI_CALL) {
        auto* call = reinterpret_cast<std::shared_ptr<MainThreadCall>*>(lParam);
        (*call)->run();
        delete call;
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

Window* GetWindow(HWND hWnd) {
    return (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

} // namespace

void quitMainLoop() {
    PostQuitMessage(0);
}

void GuiModule::startPlatform() {
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

bool GuiModule::post(const std::shared_ptr<MainThreadCall>& call) {
    if (!dispatcherWindow) return false;
    auto* message = new std::shared_ptr<MainThreadCall>(call);
    if (!PostMessage(dispatcherWindow, WM_LUWOW_GUI_CALL, 0, reinterpret_cast<LPARAM>(message))) {
        delete message;
        return false;
    }
    return true;
}

void GuiModule::runLoop() {
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

} // namespace Luwow::Gui
