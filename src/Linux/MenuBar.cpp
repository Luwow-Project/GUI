#include "MenuBar.h"
#include "Menu.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

MenuBar::MenuBar(IWindow* parent) {
    Window* window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    menuBar = Gtk::manage(new Gtk::MenuBar());
    window->setMenuBar(*menuBar);
}

// Its menus are gone first
MenuBar::~MenuBar() {
    delete menuBar;
}

void MenuBar::insertMenu(size_t index, IMenu* menu) {
    Gtk::MenuItem* title = static_cast<Menu*>(menu)->getTitleItem();
    menuBar->insert(*title, (int)index);
    title->show();
}

} // namespace Luwow::Gui
