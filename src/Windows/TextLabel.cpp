#include <windows.h>
#include "TextLabel.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

TextLabel::TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    HWND hParent = window->getHWND();
    hLabel = CreateWindowExA(
        0,
        "STATIC",
        descriptor.Text.c_str(),
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height,
        hParent,
        NULL,
        (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE),
        NULL
    );
    if (!hLabel) {
        throw std::runtime_error("Failed to create text label");
    }
}

TextLabel::~TextLabel() {
    if (window->isOpen()) DestroyWindow(hLabel);
}

void TextLabel::setText(const std::string& text) {
    if (window->isOpen()) SetWindowTextA(hLabel, text.c_str());
}

void TextLabel::setBounds(int left, int top, int width, int height) {
    if (window->isOpen()) MoveWindow(hLabel, left, top, width, height, TRUE);
}

} // namespace Luwow::Gui
