#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A menu of a menu bar, with menu items as its children
class Menu : public Control {
public:
    static constexpr const char* TypeName = "Menu";
    static const Properties<Menu>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    bool takesChildren() const override { return true; }
    bool acceptsChild(const Control& child) const override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }

    MenuDescriptor properties;
    std::unique_ptr<IMenu> native;
};

} // namespace Luwow::Gui::ControlNode
