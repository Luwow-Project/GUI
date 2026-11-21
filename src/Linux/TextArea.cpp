#include "TextArea.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

TextArea::TextArea(const TextAreaDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    textView = Gtk::manage(new Gtk::TextView());
    textView->get_buffer()->set_text(descriptor.Text);
    textView->set_wrap_mode(Gtk::WRAP_WORD_CHAR);

    scrolled = Gtk::manage(new Gtk::ScrolledWindow());
    scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled->add(*textView);
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        scrolled->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*scrolled, descriptor.Left, descriptor.Top);
    scrolled->show_all();

    changed = textView->get_buffer()->signal_changed().connect([this] {
        if (!updating && onTextChanged) onTextChanged();
    });
    auto scroll = [this] {
        if (onScrolled) onScrolled();
    };
    scrolledVertically = scrolled->get_vadjustment()->signal_value_changed().connect(scroll);
    scrolledHorizontally = scrolled->get_hadjustment()->signal_value_changed().connect(scroll);
}

TextArea::~TextArea() {
    changed.disconnect();
    scrolledVertically.disconnect();
    scrolledHorizontally.disconnect();
    delete scrolled;
}

void TextArea::setText(const std::string& text) {
    updating = true;
    textView->get_buffer()->set_text(text);
    updating = false;
}

std::string TextArea::getText() const {
    return textView->get_buffer()->get_text();
}

void TextArea::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*scrolled, left, top);
    if (width > 0 && height > 0) {
        scrolled->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
