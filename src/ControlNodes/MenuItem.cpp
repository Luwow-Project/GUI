#include "MenuItem.h"
#include "Handle.h"
#include "Menu.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

const Properties<MenuItem>& MenuItem::GetProperties() {
    static const Properties<MenuItem> properties = {
        stringProperty<MenuItem, &MenuItemDescriptor::Title>("Title", [](MenuItem& item) {
            item.native->setTitle(item.properties.Title);
        }),
    };
    return properties;
}

void MenuItem::pushHandle(lua_State* L) {
    Handle<MenuItem>::Push(L, shared_from_this());
}

void MenuItem::readProperties(lua_State* L, int index) {
    readDescriptor<MenuItem>(L, index, properties);
}

void MenuItem::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    native.reset(module->createMenuItem(properties, Window::containing(*parent)));
    static_cast<Menu*>(parent)->native->insertItem(position, native.get());
}

} // namespace Luwow::Gui::ControlNode
