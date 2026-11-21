#include <windows.h>
#include "Checkbox.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

Checkbox::Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = window->registerControl(this);

    HWND hParent = window->getHWND();
    hCheck = CreateWindowExA(
        0,
        "BUTTON",
        descriptor.Caption.c_str(),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height,
        hParent,
        (HMENU)(UINT_PTR)id,
        (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE),
        NULL
    );
    if (!hCheck) {
        window->unregisterControl(id);
        throw std::runtime_error("Failed to create check box");
    }
    setChecked(descriptor.Checked);
}

Checkbox::~Checkbox() {
    window->unregisterControl(id);
    if (window->isOpen()) DestroyWindow(hCheck);
}

void Checkbox::setCaption(const std::string& caption) {
    if (window->isOpen()) SetWindowTextA(hCheck, caption.c_str());
}

void Checkbox::setChecked(bool checked) {
    if (window->isOpen()) SendMessageA(hCheck, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool Checkbox::isChecked() const {
    return window->isOpen() && SendMessageA(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void Checkbox::setBounds(int left, int top, int width, int height) {
    if (window->isOpen()) MoveWindow(hCheck, left, top, width, height, TRUE);
}

} // namespace Luwow::Gui
