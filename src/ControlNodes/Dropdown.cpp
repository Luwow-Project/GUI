#include "Dropdown.h"
#include "Handle.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(Dropdown& dropdown) {
    const DropdownDescriptor& p = dropdown.properties;
    dropdown.native->setBounds(p.Left, p.Top, p.Width, p.Height);
}

void applySelected(Dropdown& dropdown) {
    dropdown.native->setSelected(dropdown.properties.SelectedIndex - 1);
}

void applyItems(Dropdown& dropdown) {
    dropdown.native->setItems(dropdown.properties.Items);
    applySelected(dropdown);
}

// Items, a list of strings. An item selected past the new list's end is no longer selected.
Property<Dropdown> itemsProperty() {
    return {
        "Items", "a list of strings", false,
        [](lua_State* L, int index, DropdownDescriptor& descriptor) {
            if (!lua_istable(L, index)) return false;
            std::vector<std::string> items;
            int count = (int)lua_objlen(L, index);
            for (int i = 1; i <= count; ++i) {
                lua_rawgeti(L, index, i);
                bool isString = lua_type(L, -1) == LUA_TSTRING;
                if (isString) items.emplace_back(lua_tostring(L, -1));
                lua_pop(L, 1);
                if (!isString) return false;
            }
            descriptor.Items = std::move(items);
            if (descriptor.SelectedIndex > (int)descriptor.Items.size()) descriptor.SelectedIndex = 0;
            return true;
        },
        [](lua_State* L, const DropdownDescriptor& descriptor) {
            lua_createtable(L, (int)descriptor.Items.size(), 0);
            for (size_t i = 0; i < descriptor.Items.size(); ++i) {
                lua_pushlstring(L, descriptor.Items[i].data(), descriptor.Items[i].size());
                lua_rawseti(L, -2, (int)i + 1);
            }
        },
        [](const DropdownDescriptor& a, const DropdownDescriptor& b) { return a.Items == b.Items; },
        applyItems,
    };
}

// SelectedIndex, the 1-based position of the item selected, 0 for none. Items comes before it, so a descriptor's
// is checked against its own Items.
Property<Dropdown> selectedIndexProperty() {
    return {
        "SelectedIndex", "an item's position, or 0 for none", false,
        [](lua_State* L, int index, DropdownDescriptor& descriptor) {
            if (!lua_isnumber(L, index)) return false;
            double value = lua_tonumber(L, index);
            if (value != (int)value || value < 0 || value > (double)descriptor.Items.size()) return false;
            descriptor.SelectedIndex = (int)value;
            return true;
        },
        [](lua_State* L, const DropdownDescriptor& descriptor) { lua_pushinteger(L, descriptor.SelectedIndex); },
        [](const DropdownDescriptor& a, const DropdownDescriptor& b) { return a.SelectedIndex == b.SelectedIndex; },
        applySelected,
    };
}

} // namespace

const Properties<Dropdown>& Dropdown::GetProperties() {
    static const Properties<Dropdown> properties = {
        itemsProperty(),
        selectedIndexProperty(),
        pairProperty<Dropdown, &DropdownDescriptor::Width, &DropdownDescriptor::Height>("Size", applyBounds),
        pairProperty<Dropdown, &DropdownDescriptor::Left, &DropdownDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

void Dropdown::pushHandle(lua_State* L) {
    Handle<Dropdown>::Push(L, shared_from_this());
}

void Dropdown::readProperties(lua_State* L, int index) {
    readDescriptor<Dropdown>(L, index, properties);
}

void Dropdown::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    (void)position;
    native.reset(module->createDropdown(properties, Window::containing(*parent)));
    native->onSelectionChanged = syncHandler(*this);
}

void Dropdown::sync(lua_State* L) {
    DropdownDescriptor before = properties;
    properties.SelectedIndex = native->getSelected() + 1;
    fireChanges(L, *this, before);
}

} // namespace Luwow::Gui::ControlNode
