#pragma once

#include "IGuiModule.h"
#include "MenuItem.h"
#include <windows.h>
#include <memory>
#include <vector>

namespace Luwow::Gui {

class MenuBar : public IMenuBar {
public:
    MenuBar(const MenuBarDescriptor& descriptor, IWindow* parent);
    ~MenuBar();

    HMENU getMenu() const { return menu; }

private:
    HMENU menu = nullptr;
    std::vector<std::unique_ptr<MenuItem>> items;
};

void getMenuBarTable(lua_State* L, MenuBar* menuBar);

} // namespace Luwow::Gui
