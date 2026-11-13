#pragma once

#include "Node.h"
#include "Property.h"
#include "Userdata.h"
#include "lua.h"
#include "lualib.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace Luwow::Gui {

// The userdata scripts use a node of the ControlNode N through. Its properties are N's, its methods those
// every node has and N's own.
template <typename N>
class Handle : public Luwow::Engine::Userdata<Handle<N>> {
public:
    static constexpr const char* Name = N::TypeName;
    using MethodDef = typename Luwow::Engine::Userdata<Handle>::MethodDef;
    using PropertyDef = typename Luwow::Engine::Userdata<Handle>::PropertyDef;

    explicit Handle(std::shared_ptr<Node> node) : node(std::move(node)) {}

    std::shared_ptr<Node> node;

    N& self(lua_State* L) {
        if (!node->alive) luaL_error(L, "the %s was removed", Name);
        return static_cast<N&>(*node);
    }

    // A method of N's, a static function of the node
    template <int (*Method)(lua_State*, N&)>
    int invoke(lua_State* L) {
        return Method(L, self(L));
    }

    // The property named by the key
    int getProperty(lua_State* L) {
        N& node = self(L);
        find(L).push(L, node.properties);
        return 1;
    }

    // Does nothing when the value is what the property is, and otherwise gives it to the control and fires
    // the Changed signals of the properties that end up changed. The platform may adjust the value.
    int setProperty(lua_State* L) {
        N& node = self(L);
        const Property<N>& property = find(L);
        node.sync(L);
        typename Property<N>::Descriptor before = node.properties;
        if (!property.read(L, 3, node.properties)) luaL_error(L, "%s must be %s", property.name, property.expected);
        if (property.same(before, node.properties)) return 0;
        property.apply(node);
        fireChanges(L, node, before);
        return 0;
    }

    // The event's signal, named by the key
    int getEvent(lua_State* L) {
        pushSignal(L, self(L), lua_tostring(L, 2));
        return 1;
    }

    // node.Name, nil for an unnamed node
    int getName(lua_State* L) {
        if (node->name.empty()) lua_pushnil(L);
        else lua_pushlstring(L, node->name.data(), node->name.size());
        return 1;
    }

    // node.ClassName, the descriptor's Type
    int getClassName(lua_State* L) {
        lua_pushstring(L, Name);
        return 1;
    }

    // node.Parent, nil for a window
    int getParent(lua_State* L) {
        Node& node = self(L);
        if (node.parent) pushNode(L, node.parent->shared_from_this());
        else lua_pushnil(L);
        return 1;
    }

    // node:getDescription(), a descriptor of the node as it is now, its children's included
    int getDescription(lua_State* L) {
        pushDescription(L, self(L));
        return 1;
    }

    // node:getChildren(), in order
    int getChildren(lua_State* L) {
        Node& node = self(L);
        lua_createtable(L, (int)node.children.size(), 0);
        for (size_t i = 0; i < node.children.size(); ++i) {
            pushNode(L, node.children[i]);
            lua_rawseti(L, -2, (int)i + 1);
        }
        return 1;
    }

    // node:bindToPropertyChanged(property), a signal fired with the new value when the property changes
    int bindToPropertyChanged(lua_State* L) {
        Node& node = self(L);
        const char* property = luaL_checkstring(L, 2);
        const auto& properties = PropertyNames();
        if (properties.find(property) == properties.end()) {
            luaL_error(L, "%s has no property '%s'", Name, property);
        }
        pushSignal(L, node, std::string("Changed:") + property);
        return 1;
    }

    // node:add(descriptor, at?), the new child, at a 1-based position or before a child, the end by default
    int add(lua_State* L) {
        Node& node = self(L);
        luaL_checktype(L, 2, LUA_TTABLE);
        size_t position = lua_isnoneornil(L, 3) ? node.children.size() : positionOf(L, node, 3, true);
        pushNode(L, buildNode(L, 2, node.module, &node, position));
        return 1;
    }

    // node:remove(child), the child itself, its name or its position, taking it and its children away
    int remove(lua_State* L) {
        Node& node = self(L);
        removeChild(L, node, positionOf(L, node, 2, false));
        return 0;
    }

    // Registers the handle class, and makes node.ChildName find a child
    static void Register(lua_State* L) {
        std::vector<MethodDef> methods = {
            { "getDescription", &Handle::getDescription },
            { "getChildren", &Handle::getChildren },
            { "bindToPropertyChanged", &Handle::bindToPropertyChanged },
        };
        // A node made only to ask what every node of N has
        N prototype(nullptr);
        if (prototype.takesChildren()) {
            methods.push_back({ "add", &Handle::add });
            methods.push_back({ "remove", &Handle::remove });
        }
        if constexpr (requires { N::template Methods<Handle>(); }) {
            for (const MethodDef& method : N::template Methods<Handle>()) methods.push_back(method);
        }

        std::vector<PropertyDef> properties = {
            { "Name", &Handle::getName, nullptr },
            { "ClassName", &Handle::getClassName, nullptr },
            { "Parent", &Handle::getParent, nullptr },
        };
        for (const Property<N>& property : N::GetProperties()) {
            properties.push_back({ property.name, &Handle::getProperty, &Handle::setProperty });
        }

        auto& members = Members();
        for (const MethodDef& method : methods) members.insert(method.name);
        for (const PropertyDef& property : properties) {
            members.insert(property.name);
            PropertyNames().insert(property.name);
        }
        if (const char* event = prototype.activationEvent()) {
            properties.push_back({ event, &Handle::getEvent, nullptr });
            members.insert(event);
        }

        Luwow::Engine::Userdata<Handle>::Register(L, methods, properties);

        // The generated __index runs when the key isn't a child's name. Methods and setters touch the
        // controls, which other threads have the main thread run.
        luaL_getmetatable(L, Name);
        lua_getfield(L, -1, "__index");
        lua_pushcclosure(L, IndexChildren, Name, 1);
        lua_setfield(L, -2, "__index");
        for (const char* metamethod : { "__namecall", "__newindex" }) {
            lua_getfield(L, -1, metamethod);
            lua_pushcclosure(L, OnMainThread, metamethod, 1);
            lua_setfield(L, -2, metamethod);
        }
        lua_pop(L, 1);
    }

    // N's properties and methods, which a child's name can't take
    static std::unordered_set<std::string>& Members() {
        static std::unordered_set<std::string> members;
        return members;
    }

    // The node of the handle of N's at index, nullptr when it's anything else
    static std::shared_ptr<Node> NodeAt(lua_State* L, int index) {
        if (!lua_isuserdata(L, index) || !lua_getmetatable(L, index)) return nullptr;
        luaL_getmetatable(L, Name);
        bool same = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        return same ? static_cast<Handle*>(lua_touserdata(L, index))->node : nullptr;
    }

private:
    // The properties bindToPropertyChanged takes
    static std::unordered_set<std::string>& PropertyNames() {
        static std::unordered_set<std::string> properties;
        return properties;
    }

    // The property named by the key at index 2, which the generated __index and __newindex found
    static const Property<N>& find(lua_State* L) {
        const char* key = lua_tostring(L, 2);
        for (const Property<N>& property : N::GetProperties()) {
            if (std::string(property.name) == key) return property;
        }
        luaL_error(L, "%s has no property '%s'", Name, key);
        return N::GetProperties().front();
    }

    static int IndexChildren(lua_State* L) {
        Handle* handle = Handle::Check(L, 1);
        if (lua_type(L, 2) == LUA_TSTRING && handle->node->alive) {
            if (std::shared_ptr<Node> child = childNamed(*handle->node, lua_tostring(L, 2))) {
                pushNode(L, child);
                return 1;
            }
        }
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        lua_call(L, 2, 1);
        // node.method, for calls without the colon
        if (lua_iscfunction(L, -1)) lua_pushcclosure(L, OnMainThread, "method", 1);
        return 1;
    }

    static int OnMainThread(lua_State* L) {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        return callFunctionOnMainThread(L);
    }
};

} // namespace Luwow::Gui
