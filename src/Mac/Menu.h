#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

// A menu of the menu bar: a submenu of items, shown through its item in the bar
class Menu : public IMenu {
public:
    explicit Menu(const MenuDescriptor& descriptor);
    ~Menu() override;

    void setTitle(const std::string& title) override;
    void insertItem(size_t index, IMenuItem* item) override;

    // The NSMenuItem the bar shows the menu with
    void* getNativeTopItem() const { return nativeTopItem; }

private:
    void* nativeSubmenu = nullptr;
    void* nativeTopItem = nullptr;
};

} // namespace Luwow::Gui
