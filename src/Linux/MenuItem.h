#pragma once

#include "IGuiModule.h"

#include <gtkmm/menuitem.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override;

    void setTitle(const std::string& title) override;

    Gtk::MenuItem* getWidget() const { return item; }

private:
    Gtk::MenuItem* item = nullptr;
    sigc::connection activate;
};

} // namespace Luwow::Gui
