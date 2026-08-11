#include "MenuBar.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

MenuBar::MenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    menu = CreateMenu();
    if (!menu) {
        throw std::runtime_error("Failed to create menu bar");
    }

    for (const MenuDescriptor& menuDescriptor : descriptor.Menus) {
        HMENU popup = CreatePopupMenu();
        if (!popup) {
            throw std::runtime_error("Failed to create submenu");
        }

        for (const MenuItemDescriptor& itemDescriptor : menuDescriptor.Items) {
            auto item = std::make_unique<MenuItem>(itemDescriptor, parentWindow);
            if (!AppendMenuA(popup, MF_STRING, item->getId(), itemDescriptor.Title.c_str())) {
                throw std::runtime_error("Failed to append menu item");
            }
            items.push_back(std::move(item));
        }

        if (!AppendMenuA(menu, MF_POPUP, (UINT_PTR)popup, menuDescriptor.Title.c_str())) {
            throw std::runtime_error("Failed to append menu");
        }
    }

    if (!SetMenu(parentWindow->getHWND(), menu)) {
        throw std::runtime_error("Failed to assign menu bar to window");
    }

    DrawMenuBar(parentWindow->getHWND());
}

MenuBar::~MenuBar() {
    if (menu) {
        DestroyMenu(menu);
        menu = nullptr;
    }
}

void getMenuBarTable(lua_State* L, MenuBar* menuBar) {
    lua_createtable(L, 0, 0);
    lua_pushlightuserdata(L, menuBar);
    lua_setfield(L, -2, "menuBar");
    lua_setreadonly(L, -1, 1);
}

} // namespace Luwow::Gui
