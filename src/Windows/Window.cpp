#include "Window.h"
#include <stdexcept>
#include <string>

namespace Luwow::Gui {

Window::Window(const WindowDescriptor& descriptor) {
    hWnd = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "LuwowWindow",
        descriptor.Title.c_str(),
        WS_OVERLAPPEDWINDOW,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hWnd == NULL) {
        throw std::runtime_error("Failed to create window");
    }

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

uint16_t Window::registerCommandControl(ICommandControl* commandControl) {
    uint16_t id = nextCommandId++;
    commandControls[id] = commandControl;
    return id;
}

void Window::unregisterCommandControl(uint16_t id) {
    commandControls.erase(id);
}

ICommandControl* Window::getCommandControl(uint16_t id) const {
    auto it = commandControls.find(id);
    return it == commandControls.end() ? nullptr : it->second;
}

void Window::show() {
    if (!hWnd) return;
    if (!shown) {
        shown = true;
        windowShown();
    }
    ShowWindow(hWnd, SW_SHOW);
}

void Window::destroyed() {
    hWnd = NULL;
    if (shown) {
        shown = false;
        windowClosed();
    }
}

void Window::close() {
    if (hWnd) DestroyWindow(hWnd);
}

void Window::setTitle(const std::string& title) {
    if (hWnd) SetWindowTextA(hWnd, title.c_str());
}

void Window::setBounds(int left, int top, int width, int height) {
    if (hWnd) MoveWindow(hWnd, left, top, width, height, TRUE);
}

// A minimized window's bounds are the ones it's restored to
void Window::getBounds(int& left, int& top, int& width, int& height) const {
    if (!hWnd) return;
    RECT rect = {};
    if (IsIconic(hWnd)) {
        WINDOWPLACEMENT placement = {};
        placement.length = sizeof(WINDOWPLACEMENT);
        if (!GetWindowPlacement(hWnd, &placement)) return;
        rect = placement.rcNormalPosition;
    } else if (!GetWindowRect(hWnd, &rect)) {
        return;
    }
    left = rect.left;
    top = rect.top;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}

Window::~Window() {
    if (hWnd) {
        HWND destroying = hWnd;
        SetWindowLongPtr(destroying, GWLP_USERDATA, 0);
        destroyed();
        DestroyWindow(destroying);
    }
}

} // namespace Luwow::Gui
