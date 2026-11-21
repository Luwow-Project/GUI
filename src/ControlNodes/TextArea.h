#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A multi-line text field in a window
class TextArea : public Control {
public:
    static constexpr const char* TypeName = "TextArea";
    static const Properties<TextArea>& GetProperties();

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }
    void sync(lua_State* L) override;

    TextAreaDescriptor properties;
    std::unique_ptr<ITextArea> native;
};

} // namespace Luwow::Gui::ControlNode
