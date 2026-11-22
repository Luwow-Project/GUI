#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A static text label in a window
class TextLabel : public Control {
public:
    static constexpr const char* TypeName = "TextLabel";
    static const Properties<TextLabel>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }

    TextLabelDescriptor properties;
    std::unique_ptr<ITextLabel> native;
};

} // namespace Luwow::Gui::ControlNode
