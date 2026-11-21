#include <windows.h>
#include "Dropdown.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

namespace {

// A combo box's height takes in its list, so the list opens this far below the height asked for
constexpr int ListHeight = 200;

} // namespace

Dropdown::Dropdown(const DropdownDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = window->registerControl(this);

    HWND hParent = window->getHWND();
    hCombo = CreateWindowExA(
        0,
        "COMBOBOX",
        "",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST,
        descriptor.Left,
        descriptor.Top,
        descriptor.Width,
        descriptor.Height + ListHeight,
        hParent,
        (HMENU)(UINT_PTR)id,
        (HINSTANCE)GetWindowLongPtr(hParent, GWLP_HINSTANCE),
        NULL
    );
    if (!hCombo) {
        window->unregisterControl(id);
        throw std::runtime_error("Failed to create dropdown");
    }
    setItems(descriptor.Items);
    setSelected(descriptor.SelectedIndex - 1);
}

Dropdown::~Dropdown() {
    window->unregisterControl(id);
    if (window->isOpen()) DestroyWindow(hCombo);
}

void Dropdown::setItems(const std::vector<std::string>& items) {
    if (!window->isOpen()) return;
    SendMessageA(hCombo, CB_RESETCONTENT, 0, 0);
    for (const std::string& item : items) SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)item.c_str());
}

void Dropdown::setSelected(int index) {
    if (window->isOpen()) SendMessageA(hCombo, CB_SETCURSEL, (WPARAM)index, 0);
}

int Dropdown::getSelected() const {
    if (!window->isOpen()) return -1;
    LRESULT index = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
    return index == CB_ERR ? -1 : (int)index;
}

void Dropdown::setBounds(int left, int top, int width, int height) {
    if (window->isOpen()) MoveWindow(hCombo, left, top, width, height + ListHeight, TRUE);
}

} // namespace Luwow::Gui
