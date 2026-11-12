#include "MenuBar.h"
#include "Menu.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

MenuBar::MenuBar(IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    menu = CreateMenu();
    if (!menu) {
        throw std::runtime_error("Failed to create menu bar");
    }
    if (!SetMenu(window->getHWND(), menu)) {
        DestroyMenu(menu);
        throw std::runtime_error("Failed to assign menu bar to window");
    }
    DrawMenuBar(window->getHWND());
}

MenuBar::~MenuBar() {
    if (!window->isOpen()) return;
    SetMenu(window->getHWND(), NULL);
    DestroyMenu(menu);
    DrawMenuBar(window->getHWND());
}

void MenuBar::insertMenu(size_t index, IMenu* newMenu) {
    Menu* added = static_cast<Menu*>(newMenu);
    if (!InsertMenuA(menu, (UINT)index, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT_PTR)added->getPopup(), added->getTitle().c_str())) {
        throw std::runtime_error("Failed to add a menu");
    }
    added->attach(menu, window);
    DrawMenuBar(window->getHWND());
}

} // namespace Luwow::Gui
