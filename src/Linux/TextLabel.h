#pragma once

#include "IGuiModule.h"

#include <gtkmm/label.h>

namespace Luwow::Gui {

class Window;

class TextLabel : public ITextLabel {
public:
    TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent);
    ~TextLabel() override;
    void* getNativeHandle() const override { return label->gobj(); }

    void setText(const std::string& text) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::Label* label = nullptr;
};

} // namespace Luwow::Gui
