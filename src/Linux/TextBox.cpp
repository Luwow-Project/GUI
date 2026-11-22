#include "TextBox.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

TextBox::TextBox(const TextBoxDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    entry = Gtk::manage(new Gtk::Entry());
    entry->set_text(descriptor.Text);
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        entry->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*entry, descriptor.Left, descriptor.Top);
    entry->show();

    changed = entry->signal_changed().connect([this] {
        if (!updating && onTextChanged) onTextChanged();
    });
    scrolled = entry->property_scroll_offset().signal_changed().connect([this] {
        if (onScrolled) onScrolled();
    });
}

TextBox::~TextBox() {
    changed.disconnect();
    scrolled.disconnect();
    delete entry;
}

void TextBox::setText(const std::string& text) {
    updating = true;
    entry->set_text(text);
    updating = false;
}

std::string TextBox::getText() const {
    return entry->get_text();
}

void TextBox::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*entry, left, top);
    if (width > 0 && height > 0) {
        entry->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
