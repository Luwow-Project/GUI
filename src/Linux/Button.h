#pragma once

#include "IGuiModule.h"

#include <gtkmm/button.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Button : public IButton {
public:
    Button(const ButtonDescriptor& descriptor, IWindow* parent);
    ~Button() override;

    void onCommand() override;

private:
    ButtonDescriptor descriptor;
    Gtk::Button* button = nullptr;
    sigc::connection clicked;
};

} // namespace Luwow::Gui
