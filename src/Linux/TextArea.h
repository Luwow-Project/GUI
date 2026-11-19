#pragma once

#include "IGuiModule.h"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Window;

class TextArea : public ITextArea {
public:
    TextArea(const TextAreaDescriptor& descriptor, IWindow* parent);
    ~TextArea() override;

    void setText(const std::string& text) override;
    std::string getText() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::ScrolledWindow* scrolled = nullptr;
    Gtk::TextView* textView = nullptr;
    sigc::connection changed;
    bool updating = false;
};

} // namespace Luwow::Gui
