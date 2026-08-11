#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override;

    void* getNativeMenuItem() const { return nativeMenuItem; }
    void onCommand() override;

private:
    MenuItemDescriptor descriptor;
    void* nativeMenuItem = nullptr;
    void* nativeTarget = nullptr;
};

} // namespace Luwow::Gui
