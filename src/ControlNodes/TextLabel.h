#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A static text label in a window
class TextLabel : public Node {
public:
    static constexpr const char* TypeName = "TextLabel";
    static const Properties<TextLabel>& GetProperties();

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    void readProperties(lua_State* L, int index) override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }

    TextLabelDescriptor properties;
    std::unique_ptr<ITextLabel> control;
};

} // namespace Luwow::Gui::ControlNode
