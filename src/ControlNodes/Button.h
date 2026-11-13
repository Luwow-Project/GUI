#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A push button in a window
class Button : public Node {
public:
    static constexpr const char* TypeName = "Button";
    static const Properties<Button>& GetProperties();

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    const std::unordered_set<std::string>& memberNames() const override;
    void pushHandle(lua_State* L) override;
    const char* activationEvent() const override { return "OnPressed"; }
    IActivatable* activatable() override { return control.get(); }
    void readProperties(lua_State* L, int index) override;
    void describe(lua_State* L) const override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }

    ButtonDescriptor properties;
    std::unique_ptr<IButton> control;
};

} // namespace Luwow::Gui::ControlNode
