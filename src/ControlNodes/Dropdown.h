#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A list of items in a window, the user picking one
class Dropdown : public Control {
public:
    static constexpr const char* TypeName = "Dropdown";
    static const Properties<Dropdown>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }
    void sync(lua_State* L) override;

    DropdownDescriptor properties;
    std::unique_ptr<IDropdown> native;
};

} // namespace Luwow::Gui::ControlNode
