#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A single-line text field in a window
class TextBox : public Control {
public:
    static constexpr const char* TypeName = "TextBox";
    static const Properties<TextBox>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }
    void sync(lua_State* L) override;

    TextBoxDescriptor properties;
    std::unique_ptr<ITextBox> native;
};

} // namespace Luwow::Gui::ControlNode
