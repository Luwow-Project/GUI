#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// An item of a menu
class MenuItem : public Node {
public:
    static constexpr const char* TypeName = "MenuItem";
    static const Properties<MenuItem>& GetProperties();

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    const std::unordered_set<std::string>& memberNames() const override;
    void pushHandle(lua_State* L) override;
    const char* activationEvent() const override { return "OnSelected"; }
    IActivatable* activatable() override { return control.get(); }
    void readProperties(lua_State* L, int index) override;
    void describe(lua_State* L) const override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }

    MenuItemDescriptor properties;
    std::unique_ptr<IMenuItem> control;
};

} // namespace Luwow::Gui::ControlNode
