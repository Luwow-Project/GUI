#pragma once

#include "Control.h"
#include "Property.h"
#include "Userdata.h"
#include "lua.h"
#include "lualib.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace Luwow::Gui {

// The userdata scripts use a control of the ControlNode N through. Its properties are N's, its methods those
// every control has and N's own.
template <typename N>
class Handle : public Luwow::Engine::Userdata<Handle<N>> {
public:
    static constexpr const char* Name = N::TypeName;
    using MethodDef = typename Luwow::Engine::Userdata<Handle>::MethodDef;
    using PropertyDef = typename Luwow::Engine::Userdata<Handle>::PropertyDef;

    explicit Handle(std::shared_ptr<Control> control) : control(std::move(control)) {}

    std::shared_ptr<Control> control;

    N& self(lua_State* L) {
        if (!control->alive) luaL_error(L, "the %s was removed", Name);
        return static_cast<N&>(*control);
    }

    // A method of N's, a static function of the control
    template <int (*Method)(lua_State*, N&)>
    int invoke(lua_State* L) {
        return Method(L, self(L));
    }

    // The property named by the key
    int getProperty(lua_State* L) {
        N& control = self(L);
        find(L).push(L, control.properties);
        return 1;
    }

    // Does nothing when the value is what the property is, and otherwise gives it to the native control and fires
    // the Changed signals of the properties that end up changed. The platform may adjust the value. A
    // detached control keeps it for the native control it gets.
    int setProperty(lua_State* L) {
        N& control = self(L);
        const Property<N>& property = find(L);
        if (control.native) control.sync(L);
        typename Property<N>::Descriptor before = control.properties;
        if (!property.read(L, 3, control.properties)) luaL_error(L, "%s must be %s", property.name, property.expected);
        if (property.same(before, control.properties)) return 0;
        if (control.native) property.apply(control);
        fireChanges(L, control, before);
        return 0;
    }

    // The event's signal, named by the key
    int getEvent(lua_State* L) {
        pushSignal(L, self(L), lua_tostring(L, 2));
        return 1;
    }

    // control.Name, nil for an unnamed control
    int getName(lua_State* L) {
        if (control->name.empty()) lua_pushnil(L);
        else lua_pushlstring(L, control->name.data(), control->name.size());
        return 1;
    }

    // control.ClassName, the descriptor's Type
    int getClassName(lua_State* L) {
        lua_pushstring(L, Name);
        return 1;
    }

    // control.Parent, nil for a window
    int getParent(lua_State* L) {
        Control& control = self(L);
        if (control.parent) pushControl(L, control.parent->shared_from_this());
        else lua_pushnil(L);
        return 1;
    }

    // control:getChildren(), in order
    int getChildren(lua_State* L) {
        Control& control = self(L);
        lua_createtable(L, (int)control.children.size(), 0);
        for (size_t i = 0; i < control.children.size(); ++i) {
            pushControl(L, control.children[i]);
            lua_rawseti(L, -2, (int)i + 1);
        }
        return 1;
    }

    // control:getChildrenOfType(type), the children of that Type, in order
    int getChildrenOfType(lua_State* L) {
        Control& control = self(L);
        const char* type = luaL_checkstring(L, 2);
        if (!isControlType(type)) luaL_error(L, "'%s' isn't a type of control, expected %s", type, controlTypeNames().c_str());
        lua_createtable(L, 0, 0);
        int count = 0;
        for (const std::shared_ptr<Control>& child : control.children) {
            if (std::string(child->typeName()) != type) continue;
            pushControl(L, child);
            lua_rawseti(L, -2, ++count);
        }
        return 1;
    }

    // control:bindToPropertyChanged(property), a signal fired with the new value when the property changes
    int bindToPropertyChanged(lua_State* L) {
        Control& control = self(L);
        const char* property = luaL_checkstring(L, 2);
        const auto& properties = PropertyNames();
        if (properties.find(property) == properties.end()) {
            luaL_error(L, "%s has no property '%s'", Name, property);
        }
        pushSignal(L, control, std::string("Changed:") + property);
        return 1;
    }

    // control:add(child, at?), a descriptor or a detached control, at a 1-based position or before a child, the end
    // by default. Returns the child.
    int add(lua_State* L) {
        Control& control = self(L);
        std::shared_ptr<Control> child = controlArg(L, 2);
        if (!child) luaL_checktype(L, 2, LUA_TTABLE);
        else if (!child->alive) luaL_error(L, "the %s was removed", child->typeName());
        size_t position = lua_isnoneornil(L, 3) ? control.children.size() : positionOf(L, control, 3, true);
        if (child) attachControl(L, child, control, position);
        else child = buildControl(L, 2, control.module, &control, position);
        pushControl(L, child);
        return 1;
    }

    // control:remove(child), the child itself, its name or its position, taking it and its children away
    int remove(lua_State* L) {
        Control& control = self(L);
        removeChild(L, control, positionOf(L, control, 2, false));
        return 0;
    }

    // Registers the handle class
    static void Register(lua_State* L) {
        std::vector<MethodDef> methods = {
            { "getChildren", &Handle::getChildren },
            { "getChildrenOfType", &Handle::getChildrenOfType },
            { "bindToPropertyChanged", &Handle::bindToPropertyChanged },
        };
        // A control made only to ask what every control of N has
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

        for (const PropertyDef& property : properties) PropertyNames().insert(property.name);
        if (const char* event = prototype.commandEvent()) {
            properties.push_back({ event, &Handle::getEvent, nullptr });
        }

        Luwow::Engine::Userdata<Handle>::Register(L, methods, properties);

        // Methods and setters touch the native controls, which other threads have the main thread run
        luaL_getmetatable(L, Name);
        lua_getfield(L, -1, "__index");
        lua_pushcclosure(L, IndexMethods, Name, 1);
        lua_setfield(L, -2, "__index");
        for (const char* metamethod : { "__namecall", "__newindex" }) {
            lua_getfield(L, -1, metamethod);
            lua_pushcclosure(L, OnMainThread, metamethod, 1);
            lua_setfield(L, -2, metamethod);
        }
        lua_pop(L, 1);
    }

    // The control of the handle of N's at index, nullptr when it's anything else
    static std::shared_ptr<Control> ControlAt(lua_State* L, int index) {
        if (!lua_isuserdata(L, index) || !lua_getmetatable(L, index)) return nullptr;
        luaL_getmetatable(L, Name);
        bool same = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        return same ? static_cast<Handle*>(lua_touserdata(L, index))->control : nullptr;
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

    // The generated __index, with control.method run on the main thread like control:method()
    static int IndexMethods(lua_State* L) {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        lua_call(L, 2, 1);
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
