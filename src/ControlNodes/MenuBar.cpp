#include "MenuBar.h"
#include "Handle.h"
#include "Menu.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

const Properties<MenuBar>& MenuBar::GetProperties() {
    static const Properties<MenuBar> properties;
    return properties;
}

const std::unordered_set<std::string>& MenuBar::memberNames() const {
    return Handle<MenuBar>::Members();
}

void MenuBar::pushHandle(lua_State* L) {
    Handle<MenuBar>::Push(L, shared_from_this());
}

bool MenuBar::acceptsChild(const Node& child) const {
    return dynamic_cast<const Menu*>(&child) != nullptr;
}

void MenuBar::readProperties(lua_State* L, int index) {
    readDescriptor<MenuBar>(L, index, properties);
}

void MenuBar::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void MenuBar::createControl(lua_State* L, Node* parent, size_t position) {
    (void)position;
    for (const std::shared_ptr<Node>& sibling : parent->children) {
        if (dynamic_cast<MenuBar*>(sibling.get())) luaL_error(L, "a Window has only one MenuBar");
    }
    control.reset(module->createMenuBar(Window::containing(*parent)));
}

} // namespace Luwow::Gui::ControlNode
