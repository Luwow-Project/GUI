#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A single-line text field in a window
class TextBox : public Node {
public:
    static constexpr const char* TypeName = "TextBox";
    static const Properties<TextBox>& GetProperties();

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    const std::unordered_set<std::string>& memberNames() const override;
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void describe(lua_State* L) const override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }
    void sync(lua_State* L) override;

    TextBoxDescriptor properties;
    std::unique_ptr<ITextBox> control;
};

} // namespace Luwow::Gui::ControlNode
