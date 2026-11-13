#include "MenuItem.h"
#include "Handle.h"
#include "Menu.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

const Properties<MenuItem>& MenuItem::GetProperties() {
    static const Properties<MenuItem> properties = {
        stringProperty<MenuItem, &MenuItemDescriptor::Title>("Title", [](MenuItem& item) {
            item.control->setTitle(item.properties.Title);
        }),
    };
    return properties;
}

const std::unordered_set<std::string>& MenuItem::memberNames() const {
    return Handle<MenuItem>::Members();
}

void MenuItem::pushHandle(lua_State* L) {
    Handle<MenuItem>::Push(L, shared_from_this());
}

void MenuItem::readProperties(lua_State* L, int index) {
    readDescriptor<MenuItem>(L, index, properties);
}

void MenuItem::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void MenuItem::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    control.reset(module->createMenuItem(properties, Window::containing(*parent)));
    static_cast<Menu*>(parent)->control->insertItem(position, control.get());
}

} // namespace Luwow::Gui::ControlNode
