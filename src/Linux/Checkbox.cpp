#include "Checkbox.h"
#include "Mnemonic.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

Checkbox::Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    check = Gtk::manage(new Gtk::CheckButton(windowsMnemonicToGtk(descriptor.Caption), true));
    check->set_active(descriptor.Checked);
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        check->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*check, descriptor.Left, descriptor.Top);
    check->show();

    toggled = check->signal_toggled().connect([this] {
        if (!updating && onCommand) onCommand();
    });
}

Checkbox::~Checkbox() {
    toggled.disconnect();
    delete check;
}

void Checkbox::setCaption(const std::string& caption) {
    check->set_label(windowsMnemonicToGtk(caption));
}

void Checkbox::setChecked(bool checked) {
    updating = true;
    check->set_active(checked);
    updating = false;
}

bool Checkbox::isChecked() const {
    return check->get_active();
}

void Checkbox::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*check, left, top);
    if (width > 0 && height > 0) {
        check->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
