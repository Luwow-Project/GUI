#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override;

    void setTitle(const std::string& title) override;

    void* getNativeMenuItem() const { return nativeMenuItem; }

private:
    void* nativeMenuItem = nullptr;
    void* nativeTarget = nullptr;
};

} // namespace Luwow::Gui
