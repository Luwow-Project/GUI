#include "Window.h"
#include <iostream>
#include <string>
#include "lua.h"
#include "lualib.h"

namespace Luwow::Gui {

Window::Window(const WindowDescriptor& descriptor) : descriptor(descriptor) {
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

ICommandControl* Window::getCommandControl(uint16_t id) const {
    return commandControls.at(id);
}

void Window::show() {
    ShowWindow(hWnd, SW_SHOW);
}

void Window::close() {
    DestroyWindow(hWnd);
    hWnd = NULL;
}

Window::~Window() {
    //DestroyWindow(hWnd);
}

} // namespace Luwow::Gui