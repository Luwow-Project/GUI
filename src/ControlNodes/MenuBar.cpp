#include "MenuBar.h"
#include "Handle.h"
#include "Menu.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

const Properties<MenuBar>& MenuBar::GetProperties() {
    static const Properties<MenuBar> properties;
    return properties;
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

void MenuBar::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    (void)position;
    control.reset(module->createMenuBar(Window::containing(*parent)));
}

} // namespace Luwow::Gui::ControlNode
