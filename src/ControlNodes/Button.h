#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A push button in a window
class Button : public Control {
public:
    static constexpr const char* TypeName = "Button";
    static const Properties<Button>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    const char* commandEvent() const override { return "OnPressed"; }
    ICommandControl* commandControl() override { return native.get(); }
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }

    ButtonDescriptor properties;
    std::unique_ptr<IButton> native;
};

} // namespace Luwow::Gui::ControlNode
