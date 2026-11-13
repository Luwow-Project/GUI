#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A window's menu bar, with menus as its children. A window has one at most.
class MenuBar : public Node {
public:
    static constexpr const char* TypeName = "MenuBar";
    static const Properties<MenuBar>& GetProperties();

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

    MenuBarDescriptor properties;
    std::unique_ptr<IMenuBar> control;
};

} // namespace Luwow::Gui::ControlNode
