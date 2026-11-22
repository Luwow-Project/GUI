#include "Menu.h"
#include "Handle.h"
#include "MenuItem.h"
#include "MenuBar.h"

namespace Luwow::Gui::ControlNode {

const Properties<Menu>& Menu::GetProperties() {
    static const Properties<Menu> properties = {
        stringProperty<Menu, &MenuDescriptor::Title>("Title", [](Menu& menu) {
            menu.native->setTitle(menu.properties.Title);
        }),
    };
    return properties;
}

void Menu::pushHandle(lua_State* L) {
    Handle<Menu>::Push(L, shared_from_this());
}

bool Menu::acceptsChild(const Control& child) const {
    return dynamic_cast<const MenuItem*>(&child) != nullptr;
}

void Menu::readProperties(lua_State* L, int index) {
    readDescriptor<Menu>(L, index, properties);
}

void Menu::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    native.reset(module->createMenu(properties));
    static_cast<MenuBar*>(parent)->native->insertMenu(position, native.get());
}

} // namespace Luwow::Gui::ControlNode
