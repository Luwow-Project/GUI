#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A check box with its caption in a window
class Checkbox : public Control {
public:
    static constexpr const char* TypeName = "Checkbox";
    static const Properties<Checkbox>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }
    void sync(lua_State* L) override;

    CheckboxDescriptor properties;
    std::unique_ptr<ICheckbox> native;
};

} // namespace Luwow::Gui::ControlNode
