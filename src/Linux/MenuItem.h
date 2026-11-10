#pragma once

#include "IGuiModule.h"

#include <gtkmm/menuitem.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override;

    Gtk::MenuItem* getWidget() const { return item; }
    void onCommand() override;

private:
    MenuItemDescriptor descriptor;
    Gtk::MenuItem* item = nullptr;
    sigc::connection activate;
};

} // namespace Luwow::Gui
