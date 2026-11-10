#pragma once

#include "IGuiModule.h"
#include "MenuItem.h"

#include <gtkmm/menubar.h>
#include <memory>
#include <vector>

namespace Luwow::Gui {

class MenuBar : public IMenuBar {
public:
    MenuBar(const MenuBarDescriptor& descriptor, IWindow* parent);
    ~MenuBar() override;

private:
    Gtk::MenuBar* menuBar = nullptr;
    std::vector<std::unique_ptr<MenuItem>> items;
};

} // namespace Luwow::Gui
