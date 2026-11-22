#pragma once

#include "IGuiModule.h"

#include <gtkmm/entry.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Window;

class TextBox : public ITextBox {
public:
    TextBox(const TextBoxDescriptor& descriptor, IWindow* parent);
    ~TextBox() override;
    void* getNativeHandle() const override { return entry->gobj(); }

    void setText(const std::string& text) override;
    std::string getText() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::Entry* entry = nullptr;
    sigc::connection changed;
    sigc::connection scrolled;
    bool updating = false;
};

} // namespace Luwow::Gui
