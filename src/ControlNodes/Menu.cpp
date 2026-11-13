#include "Menu.h"
#include "Handle.h"
#include "MenuItem.h"
#include "MenuBar.h"

namespace Luwow::Gui::ControlNode {

const Properties<Menu>& Menu::GetProperties() {
    static const Properties<Menu> properties = {
        stringProperty<Menu, &MenuDescriptor::Title>("Title", [](Menu& menu) {
            menu.control->setTitle(menu.properties.Title);
        }),
    };
    return properties;
}

const std::unordered_set<std::string>& Menu::memberNames() const {
    return Handle<Menu>::Members();
}

void Menu::pushHandle(lua_State* L) {
    Handle<Menu>::Push(L, shared_from_this());
}

bool Menu::acceptsChild(const Node& child) const {
    return dynamic_cast<const MenuItem*>(&child) != nullptr;
}

void Menu::readProperties(lua_State* L, int index) {
    readDescriptor<Menu>(L, index, properties);
}

void Menu::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void Menu::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    control.reset(module->createMenu(properties));
    static_cast<MenuBar*>(parent)->control->insertMenu(position, control.get());
}

} // namespace Luwow::Gui::ControlNode
