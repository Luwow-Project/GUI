#pragma once

#include "IGuiModule.h"

#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>

namespace Luwow::Gui {

// A menu of a menu bar: the title in the bar, and the popup of items it opens
class Menu : public IMenu {
public:
    explicit Menu(const MenuDescriptor& descriptor);
    ~Menu() override;

    void setTitle(const std::string& title) override;
    void insertItem(size_t index, IMenuItem* item) override;

    Gtk::MenuItem* getTitleItem() const { return titleItem; }

private:
    Gtk::MenuItem* titleItem = nullptr;
    Gtk::Menu* popup = nullptr;
};

} // namespace Luwow::Gui
