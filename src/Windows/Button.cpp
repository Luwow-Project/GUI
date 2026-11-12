#include <windows.h>
#include "Button.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

Button::Button(const ButtonDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = window->registerCommandControl(this);

    HWND hParent = window->getHWND();
    hButton = CreateWindowExA(
        0,
        "BUTTON",
        descriptor.Caption.c_str(),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height,
        hParent,
        (HMENU)(UINT_PTR)id,
        (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE),
        NULL
    );
    if (!hButton) {
        window->unregisterCommandControl(id);
        throw std::runtime_error("Failed to create button");
    }
}

Button::~Button() {
    window->unregisterCommandControl(id);
    if (window->isOpen()) DestroyWindow(hButton);
}

void Button::setCaption(const std::string& caption) {
    if (window->isOpen()) SetWindowTextA(hButton, caption.c_str());
}

void Button::setBounds(int left, int top, int width, int height) {
    if (window->isOpen()) MoveWindow(hButton, left, top, width, height, TRUE);
}

} // namespace Luwow::Gui
