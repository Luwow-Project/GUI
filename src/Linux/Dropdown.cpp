#include "Dropdown.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

Dropdown::Dropdown(const DropdownDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    combo = Gtk::manage(new Gtk::ComboBoxText());
    setItems(descriptor.Items);
    setSelected(descriptor.SelectedIndex - 1);
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        combo->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*combo, descriptor.Left, descriptor.Top);
    combo->show();

    changed = combo->signal_changed().connect([this] {
        if (!updating && onSelectionChanged) onSelectionChanged();
    });
}

Dropdown::~Dropdown() {
    changed.disconnect();
    delete combo;
}

void Dropdown::setItems(const std::vector<std::string>& items) {
    updating = true;
    combo->remove_all();
    for (const std::string& item : items) combo->append(item);
    updating = false;
}

void Dropdown::setSelected(int index) {
    updating = true;
    combo->set_active(index);
    updating = false;
}

int Dropdown::getSelected() const {
    return combo->get_active_row_number();
}

void Dropdown::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*combo, left, top);
    if (width > 0 && height > 0) {
        combo->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
