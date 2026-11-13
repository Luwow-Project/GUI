#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A menu of a menu bar, with menu items as its children
class Menu : public Node {
public:
    static constexpr const char* TypeName = "Menu";
    static const Properties<Menu>& GetProperties();

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    const std::unordered_set<std::string>& memberNames() const override;
    void pushHandle(lua_State* L) override;
    bool takesChildren() const override { return true; }
    bool acceptsChild(const Node& child) const override;
    void readProperties(lua_State* L, int index) override;
    void describe(lua_State* L) const override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }

    MenuDescriptor properties;
    std::unique_ptr<IMenu> control;
};

} // namespace Luwow::Gui::ControlNode
