#include "MenuItem.h"
#include "Mnemonic.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

MenuItem::MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent) {
    if (!dynamic_cast<Window*>(parent)) {
        throw std::runtime_error("Parent window is not a Window");
    }

    item = Gtk::manage(new Gtk::MenuItem(windowsMnemonicToGtk(descriptor.Title), true));
    activate = item->signal_activate().connect(sigc::mem_fun(*this, &MenuItem::onCommand));
}

// Taken out of its menu, in it or not
MenuItem::~MenuItem() {
    activate.disconnect();
    delete item;
}

void MenuItem::setTitle(const std::string& title) {
    item->set_label(windowsMnemonicToGtk(title));
}

} // namespace Luwow::Gui
