#include <windows.h>
#include "TextArea.h"
#include "Window.h"
#include <stdexcept>
#include <vector>

namespace Luwow::Gui {

TextArea::TextArea(const TextAreaDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = window->registerControl(this);

    HWND hParent = window->getHWND();
    hEdit = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        "EDIT",
        descriptor.Text.c_str(),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_LEFT,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height,
        hParent,
        (HMENU)(UINT_PTR)id,
        (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE),
        NULL
    );
    if (!hEdit) {
        window->unregisterControl(id);
        throw std::runtime_error("Failed to create text area");
    }
}

TextArea::~TextArea() {
    window->unregisterControl(id);
    if (window->isOpen()) DestroyWindow(hEdit);
}

void TextArea::setText(const std::string& text) {
    if (!window->isOpen()) return;
    updating = true;
    SetWindowTextA(hEdit, text.c_str());
    updating = false;
}

std::string TextArea::getText() const {
    if (!window->isOpen()) return {};
    int length = GetWindowTextLengthA(hEdit);
    if (length <= 0) return {};
    std::vector<char> buffer(static_cast<size_t>(length) + 1);
    GetWindowTextA(hEdit, buffer.data(), length + 1);
    return std::string(buffer.data(), static_cast<size_t>(length));
}

void TextArea::setBounds(int left, int top, int width, int height) {
    if (window->isOpen()) MoveWindow(hEdit, left, top, width, height, TRUE);
}

} // namespace Luwow::Gui
