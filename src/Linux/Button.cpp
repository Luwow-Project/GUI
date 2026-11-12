#include "Button.h"
#include "Mnemonic.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

Button::Button(const ButtonDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    button = Gtk::manage(new Gtk::Button(windowsMnemonicToGtk(descriptor.Caption), true));
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        button->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*button, descriptor.Left, descriptor.Top);
    button->show();

    clicked = button->signal_clicked().connect(sigc::mem_fun(*this, &Button::onCommand));
}

// Taken out of its window, which stays until its children are gone
Button::~Button() {
    clicked.disconnect();
    delete button;
}

void Button::setCaption(const std::string& caption) {
    button->set_label(windowsMnemonicToGtk(caption));
}

void Button::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*button, left, top);
    if (width > 0 && height > 0) {
        button->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
