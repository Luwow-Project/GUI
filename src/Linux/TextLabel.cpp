#include "TextLabel.h"
#include "Window.h"

#include <stdexcept>

namespace Luwow::Gui {

TextLabel::TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    label = Gtk::manage(new Gtk::Label(descriptor.Text));
    label->set_xalign(0.0f);
    if (descriptor.Width > 0 && descriptor.Height > 0) {
        label->set_size_request(descriptor.Width, descriptor.Height);
    }
    window->getFixed()->put(*label, descriptor.Left, descriptor.Top);
    label->show();
}

TextLabel::~TextLabel() {
    delete label;
}

void TextLabel::setText(const std::string& text) {
    label->set_text(text);
}

void TextLabel::setBounds(int left, int top, int width, int height) {
    window->getFixed()->move(*label, left, top);
    if (width > 0 && height > 0) {
        label->set_size_request(width, height);
    }
}

} // namespace Luwow::Gui
