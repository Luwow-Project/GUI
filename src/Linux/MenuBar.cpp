#include "MenuBar.h"
#include "Mnemonic.h"
#include "Window.h"

#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <stdexcept>

namespace Luwow::Gui {

MenuBar::MenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    menuBar = Gtk::manage(new Gtk::MenuBar());

    for (const MenuDescriptor& menuDescriptor : descriptor.Menus) {
        auto* menu = Gtk::manage(new Gtk::Menu());
        auto* title = Gtk::manage(new Gtk::MenuItem(windowsMnemonicToGtk(menuDescriptor.Title), true));
        title->set_submenu(*menu);

        for (const MenuItemDescriptor& itemDescriptor : menuDescriptor.Items) {
            auto item = std::make_unique<MenuItem>(itemDescriptor, parentWindow);
            menu->append(*item->getWidget());
            items.push_back(std::move(item));
        }

        menuBar->append(*title);
    }

    parentWindow->setMenuBar(*menuBar);
}

MenuBar::~MenuBar() {
    items.clear();
}

} // namespace Luwow::Gui
