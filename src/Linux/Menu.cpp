#include "Menu.h"
#include "MenuItem.h"
#include "Mnemonic.h"

namespace Luwow::Gui {

Menu::Menu(const MenuDescriptor& descriptor) {
    popup = Gtk::manage(new Gtk::Menu());
    titleItem = Gtk::manage(new Gtk::MenuItem(windowsMnemonicToGtk(descriptor.Title), true));
    titleItem->set_submenu(*popup);
}

// Its items are gone first. The popup goes with the title it's attached to, in the bar or not.
Menu::~Menu() {
    delete titleItem;
}

void Menu::setTitle(const std::string& title) {
    titleItem->set_label(windowsMnemonicToGtk(title));
}

void Menu::insertItem(size_t index, IMenuItem* item) {
    Gtk::MenuItem* widget = static_cast<MenuItem*>(item)->getWidget();
    popup->insert(*widget, (int)index);
    widget->show();
}

} // namespace Luwow::Gui
