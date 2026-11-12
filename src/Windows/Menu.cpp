#include "Menu.h"
#include "MenuItem.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

Menu::Menu(const MenuDescriptor& descriptor) : title(descriptor.Title) {
    popup = CreatePopupMenu();
    if (!popup) {
        throw std::runtime_error("Failed to create menu");
    }
}

Menu::~Menu() {
    if (!bar) {
        DestroyMenu(popup);
        return;
    }
    // A closed window took its menus with it
    if (!window->isOpen()) return;
    int position = positionInBar();
    if (position >= 0) RemoveMenu(bar, (UINT)position, MF_BYPOSITION);
    DestroyMenu(popup);
    DrawMenuBar(window->getHWND());
}

void Menu::attach(HMENU bar, Window* window) {
    this->bar = bar;
    this->window = window;
}

int Menu::positionInBar() const {
    int count = GetMenuItemCount(bar);
    for (int i = 0; i < count; ++i) {
        if (GetSubMenu(bar, i) == popup) return i;
    }
    return -1;
}

void Menu::setTitle(const std::string& title) {
    this->title = title;
    if (!bar || !window->isOpen()) return;
    int position = positionInBar();
    if (position < 0) return;
    ModifyMenuA(bar, (UINT)position, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)popup, title.c_str());
    DrawMenuBar(window->getHWND());
}

void Menu::insertItem(size_t index, IMenuItem* item) {
    MenuItem* menuItem = static_cast<MenuItem*>(item);
    if (!InsertMenuA(popup, (UINT)index, MF_BYPOSITION | MF_STRING, menuItem->getId(), menuItem->getTitle().c_str())) {
        throw std::runtime_error("Failed to add a menu item");
    }
    menuItem->attach(popup);
}

} // namespace Luwow::Gui
