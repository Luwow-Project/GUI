#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// An item of a menu
class MenuItem : public Control {
public:
    static constexpr const char* TypeName = "MenuItem";
    static const Properties<MenuItem>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    const char* commandEvent() const override { return "OnSelected"; }
    ICommandControl* commandControl() override { return native.get(); }
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }

    MenuItemDescriptor properties;
    std::unique_ptr<IMenuItem> native;
};

} // namespace Luwow::Gui::ControlNode
