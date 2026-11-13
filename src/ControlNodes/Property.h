#pragma once

#include "Node.h"
#include "Descriptors.h"
#include "lua.h"
#include "lualib.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Luwow::Gui {

// A property of the ControlNode N, kept in a field of its descriptor. N's list of them is all there
// is to its properties: reading its descriptor, describing it, and the handle's getters and setters.
template <typename N>
struct Property {
    using Descriptor = std::remove_cvref_t<decltype(std::declval<N&>().properties)>;

    const char* name;
    // What the value must be, for errors
    const char* expected;
    // Whether a descriptor must have it
    bool required;
    // Reads the value at index into the descriptor, false when it isn't the property's type
    bool (*read)(lua_State* L, int index, Descriptor& descriptor);
    void (*push)(lua_State* L, const Descriptor& descriptor);
    bool (*same)(const Descriptor& a, const Descriptor& b);
    // Gives the node's control the value set
    void (*apply)(N& node);
};

template <typename N>
using Properties = std::vector<Property<N>>;

// A required string
template <typename N, auto Field>
Property<N> stringProperty(const char* name, void (*apply)(N&)) {
    using D = typename Property<N>::Descriptor;
    return {
        name, "a string", true,
        [](lua_State* L, int index, D& descriptor) {
            if (!lua_isstring(L, index)) return false;
            descriptor.*Field = lua_tostring(L, index);
            return true;
        },
        [](lua_State* L, const D& descriptor) {
            const std::string& value = descriptor.*Field;
            lua_pushlstring(L, value.data(), value.size());
        },
        [](const D& a, const D& b) { return a.*Field == b.*Field; },
        apply,
    };
}

// An optional {x, y} or {width, height}, a new table each time it's read
template <typename N, auto First, auto Second>
Property<N> pairProperty(const char* name, void (*apply)(N&)) {
    using D = typename Property<N>::Descriptor;
    return {
        name, "a table of two numbers", false,
        [](lua_State* L, int index, D& descriptor) {
            return readPair(L, index, descriptor.*First, descriptor.*Second);
        },
        [](lua_State* L, const D& descriptor) { pushPair(L, descriptor.*First, descriptor.*Second); },
        [](const D& a, const D& b) { return a.*First == b.*First && a.*Second == b.*Second; },
        apply,
    };
}

// Reads N's properties from the descriptor table at index
template <typename N>
void readDescriptor(lua_State* L, int index, typename Property<N>::Descriptor& descriptor) {
    for (const Property<N>& property : N::GetProperties()) {
        lua_getfield(L, index, property.name);
        if (lua_isnil(L, -1)) {
            if (property.required) luaL_error(L, "%s descriptor needs %s, %s", N::TypeName, property.name, property.expected);
        } else if (!property.read(L, -1, descriptor)) {
            luaL_error(L, "%s descriptor's %s must be %s", N::TypeName, property.name, property.expected);
        }
        lua_pop(L, 1);
    }
}

// Sets N's properties as fields of the descriptor table on top of the stack
template <typename N>
void describeProperties(lua_State* L, const N& node) {
    for (const Property<N>& property : N::GetProperties()) {
        property.push(L, node.properties);
        lua_setfield(L, -2, property.name);
    }
}

// Fires the Changed signal of each of the node's properties that's no longer what it was before
template <typename N>
void fireChanges(lua_State* L, N& node, const typename Property<N>::Descriptor& before) {
    for (const Property<N>& property : N::GetProperties()) {
        if (property.same(before, node.properties)) continue;
        property.push(L, node.properties);
        fireSignal(L, node, std::string("Changed:") + property.name, 1);
    }
}

} // namespace Luwow::Gui
