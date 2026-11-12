#pragma once

#include "IGuiModule.h"

#include <gtkmm/button.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Window;

class Button : public IButton {
public:
    Button(const ButtonDescriptor& descriptor, IWindow* parent);
    ~Button() override;

    void setCaption(const std::string& caption) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::Button* button = nullptr;
    sigc::connection clicked;
};

} // namespace Luwow::Gui
