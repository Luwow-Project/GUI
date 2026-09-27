#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override = default;

    uint16_t getId() const { return id; }
    void onCommand() override;

private:
    MenuItemDescriptor descriptor;
    uint16_t id = 0;
};

} // namespace Luwow::Gui
