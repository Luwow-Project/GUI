#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A window's menu bar, with menus as its children. A window has one at most.
class MenuBar : public Control {
public:
    static constexpr const char* TypeName = "MenuBar";
    static const Properties<MenuBar>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    bool takesChildren() const override { return true; }
    bool acceptsChild(const Control& child) const override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }

    MenuBarDescriptor properties;
    std::unique_ptr<IMenuBar> native;
};

} // namespace Luwow::Gui::ControlNode
