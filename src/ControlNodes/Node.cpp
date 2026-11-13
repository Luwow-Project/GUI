#include "Node.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Menu.h"
#include "MenuItem.h"
#include "Handle.h"
#include "ILuauHost.h"
#include "SignalCore.h"
#include "lua.h"
#include "lualib.h"

#include <algorithm>
#include <cmath>

namespace Luwow::Gui {

using Luwow::Engine::ILuauHost;

namespace {

ILuauHost* luauHost = nullptr;
IGuiModule* guiModule = nullptr;

// Each node's userdata by the node's address, weak so a handle no script holds can be collected
const char* const CacheKey = "Luwow.Gui.Nodes";

// In the order they're registered
// The ControlNodes, and what's done with each of them in turn
template <typename... ControlNodes>
struct NodeTypes {
    // A node of the descriptor Type, without its control yet, nullptr for an unknown Type
    static std::shared_ptr<Node> make(const std::string& type, IGuiModule* gui) {
        std::shared_ptr<Node> node;
        ((node = !node && type == ControlNodes::TypeName ? std::make_shared<ControlNodes>(gui) : node), ...);
        return node;
    }

    static std::shared_ptr<Node> fromHandle(lua_State* L, int index) {
        std::shared_ptr<Node> node;
        ((node = node ? node : Handle<ControlNodes>::NodeAt(L, index)), ...);
        return node;
    }

    static void registerHandles(lua_State* L) {
        (Handle<ControlNodes>::Register(L), ...);
    }

    // "A, B or C"
    static std::string names() {
        const char* list[] = { ControlNodes::TypeName... };
        std::string names;
        for (size_t i = 0; i < sizeof...(ControlNodes); ++i) {
            if (i > 0) names += i + 1 == sizeof...(ControlNodes) ? " or " : ", ";
            names += list[i];
        }
        return names;
    }
};

using AllNodeTypes = NodeTypes<ControlNode::Window, ControlNode::Button, ControlNode::MenuBar, ControlNode::Menu, ControlNode::MenuItem>;

void releaseSignals(lua_State* L, Node& node) {
    for (auto& [key, ref] : node.signals) {
        lua_getref(L, ref);
        Signals::disconnectAll(L, -1);
        lua_pop(L, 1);
        lua_unref(L, ref);
    }
    node.signals.clear();
}

// Takes the node and its children off the screen, deepest first, and ends their handles
void destroyNode(lua_State* L, Node& node) {
    for (auto it = node.children.rbegin(); it != node.children.rend(); ++it) {
        destroyNode(L, **it);
        (*it)->parent = nullptr;
    }
    node.children.clear();
    node.destroyControl();
    releaseSignals(L, node);
    node.alive = false;
}

// Builds a node's Children table at index: an array of descriptors with optional Names, and descriptors
// keyed by their names. They're ordered by Order, then the array's in its order, then the keyed by name.
void buildChildren(lua_State* L, int index, IGuiModule* gui, Node& node) {
    index = lua_absindex(L, index);
    const char* type = node.typeName();
    if (!lua_istable(L, index)) {
        luaL_error(L, "a %s's Children must be a table of descriptors", type);
    }

    struct Entry {
        double order;
        int position;
        std::string key;
    };
    auto orderAt = [L](int value) {
        if (!lua_istable(L, value)) return 0.0;
        lua_getfield(L, value, "Order");
        double order = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0.0;
        lua_pop(L, 1);
        return order;
    };

    std::vector<Entry> entries;
    int count = (int)lua_objlen(L, index);
    for (int i = 1; i <= count; ++i) {
        lua_rawgeti(L, index, i);
        entries.push_back({ orderAt(-1), i, {} });
        lua_pop(L, 1);
    }
    lua_pushnil(L);
    while (lua_next(L, index)) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            size_t length = 0;
            const char* key = lua_tolstring(L, -2, &length);
            entries.push_back({ orderAt(-1), 0, std::string(key, length) });
        } else {
            double key = lua_type(L, -2) == LUA_TNUMBER ? lua_tonumber(L, -2) : 0;
            if (key != std::floor(key) || key < 1 || key > count) {
                luaL_error(L, "a %s's Children must be an array, or keyed by the children's names", type);
            }
        }
        lua_pop(L, 1);
    }

    std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.order != b.order) return a.order < b.order;
        if ((a.position > 0) != (b.position > 0)) return a.position > 0;
        if (a.position > 0) return a.position < b.position;
        return a.key < b.key;
    });

    for (const Entry& entry : entries) {
        if (entry.position > 0) {
            lua_rawgeti(L, index, entry.position);
            buildNode(L, -1, gui, &node, node.children.size());
        } else {
            lua_pushlstring(L, entry.key.data(), entry.key.size());
            lua_rawget(L, index);
            buildNode(L, -1, gui, &node, node.children.size(), entry.key.c_str());
        }
        lua_pop(L, 1);
    }
}

// The node of a descriptor's Type, without its control yet
std::shared_ptr<Node> makeNode(lua_State* L, const std::string& type, IGuiModule* gui) {
    if (std::shared_ptr<Node> node = AllNodeTypes::make(type, gui)) return node;
    luaL_error(L, "unknown descriptor Type '%s', expected %s", type.c_str(), AllNodeTypes::names().c_str());
    return nullptr;
}

} // namespace

ILuauHost* nodeHost() {
    return luauHost;
}

// Signals

void pushSignal(lua_State* L, Node& node, const std::string& key) {
    auto it = node.signals.find(key);
    if (it != node.signals.end()) {
        lua_getref(L, it->second);
        return;
    }
    Signals::pushSignal(L);
    node.signals[key] = lua_ref(L, -1);
}

void fireSignal(lua_State* L, Node& node, const std::string& key, int nargs) {
    auto it = node.signals.find(key);
    if (it == node.signals.end()) {
        lua_pop(L, nargs);
        return;
    }
    lua_getref(L, it->second);
    lua_insert(L, -(nargs + 1));
    Signals::fireSignal(L, -(nargs + 1), nargs);
    lua_pop(L, 1);
}

// The tree

std::shared_ptr<Node> childNamed(Node& node, const char* name) {
    for (const std::shared_ptr<Node>& child : node.children) {
        if (child->name == name) return child;
    }
    return nullptr;
}

size_t positionOf(lua_State* L, Node& parent, int index, bool inserting) {
    size_t count = parent.children.size();
    if (lua_type(L, index) == LUA_TNUMBER) {
        double position = lua_tonumber(L, index);
        size_t limit = inserting ? count + 1 : count;
        if (position != std::floor(position) || position < 1 || position > (double)limit) {
            luaL_error(L, "position %g is outside 1 to %d", position, (int)limit);
        }
        return (size_t)position - 1;
    }
    if (!inserting && lua_type(L, index) == LUA_TSTRING) {
        const char* name = lua_tostring(L, index);
        for (size_t i = 0; i < count; ++i) {
            if (parent.children[i]->name == name) return i;
        }
        luaL_error(L, "the %s has no child named '%s'", parent.typeName(), name);
    }
    if (std::shared_ptr<Node> node = nodeArg(L, index)) {
        for (size_t i = 0; i < count; ++i) {
            if (parent.children[i] == node) return i;
        }
        luaL_error(L, "the %s isn't a child of this %s", node->typeName(), parent.typeName());
    }
    luaL_error(L, inserting ? "the position must be a number or a child node" : "expected a child node, its name or its position");
    return 0;
}

void removeChild(lua_State* L, Node& parent, size_t index) {
    std::shared_ptr<Node> child = parent.children[index];
    destroyNode(L, *child);
    child->parent = nullptr;
    parent.children.erase(parent.children.begin() + (std::ptrdiff_t)index);
}

int callFunctionOnMainThread(lua_State* L) {
    int nargs = lua_gettop(L) - 1;
    if (guiModule->isMainThread()) {
        lua_call(L, nargs, LUA_MULTRET);
        return lua_gettop(L);
    }
    int status = LUA_OK;
    bool ran = guiModule->runOnMainThread([&] {
        Luwow::Engine::StateLock lock(luauHost);
        status = lua_pcall(L, nargs, LUA_MULTRET, 0);
    });
    if (!ran) luaL_error(L, "gui can't be used from this thread, the main thread's windows are closed");
    if (status != LUA_OK) lua_error(L);
    return lua_gettop(L);
}

void pushNode(lua_State* L, const std::shared_ptr<Node>& node) {
    lua_getfield(L, LUA_REGISTRYINDEX, CacheKey);
    lua_pushlightuserdata(L, node.get());
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);

    node->pushHandle(L);

    lua_pushlightuserdata(L, node.get());
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    lua_remove(L, -2);
}

std::shared_ptr<Node> nodeArg(lua_State* L, int index) {
    return AllNodeTypes::fromHandle(L, index);
}

void pushDescription(lua_State* L, Node& node) {
    lua_createtable(L, 0, 8);
    lua_pushstring(L, node.typeName());
    lua_setfield(L, -2, "Type");
    if (!node.name.empty()) {
        lua_pushlstring(L, node.name.data(), node.name.size());
        lua_setfield(L, -2, "Name");
    }
    if (node.order != 0) {
        lua_pushnumber(L, node.order);
        lua_setfield(L, -2, "Order");
    }
    node.describe(L);
    if (node.takesChildren()) {
        lua_createtable(L, (int)node.children.size(), 0);
        for (size_t i = 0; i < node.children.size(); ++i) {
            pushDescription(L, *node.children[i]);
            lua_rawseti(L, -2, (int)i + 1);
        }
        lua_setfield(L, -2, "Children");
    }
}

// A node that fails partway is taken away again
std::shared_ptr<Node> buildNode(lua_State* L, int index, IGuiModule* gui, Node* parent, size_t position, const char* key) {
    index = lua_absindex(L, index);
    if (!lua_istable(L, index)) {
        luaL_error(L, "a descriptor must be a table, got %s", luaL_typename(L, index));
    }

    lua_getfield(L, index, "Type");
    if (!lua_isstring(L, -1)) {
        luaL_error(L, "a descriptor needs a Type, a string");
    }
    std::string type = lua_tostring(L, -1);
    lua_pop(L, 1);
    std::shared_ptr<Node> node = makeNode(L, type, gui);

    if (parent && !parent->acceptsChild(*node)) {
        luaL_error(L, "a %s can't be a child of a %s", type.c_str(), parent->typeName());
    }

    std::string name;
    lua_getfield(L, index, "Name");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "a %s's Name must be a string", type.c_str());
        name = lua_tostring(L, -1);
    }
    lua_pop(L, 1);
    if (key) {
        if (!*key) luaL_error(L, "a %s's key in Children can't be empty", type.c_str());
        if (!name.empty() && name != key) {
            luaL_error(L, "the %s keyed '%s' in Children is named '%s'", type.c_str(), key, name.c_str());
        }
        name = key;
    }

    double order = 0;
    lua_getfield(L, index, "Order");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) != LUA_TNUMBER) luaL_error(L, "a %s's Order must be a number", type.c_str());
        order = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
    if (parent && !name.empty()) {
        const auto& members = parent->memberNames();
        if (members.find(name) != members.end()) {
            luaL_error(L, "a child can't be named '%s', a member of %s", name.c_str(), parent->typeName());
        }
        if (childNamed(*parent, name.c_str())) {
            luaL_error(L, "the %s already has a child named '%s'", parent->typeName(), name.c_str());
        }
    }

    if (!node->takesChildren()) {
        lua_getfield(L, index, "Children");
        bool hasChildren = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (hasChildren) luaL_error(L, "%s descriptors don't take Children", type.c_str());
    }

    if (parent && !ControlNode::Window::containing(*parent)) {
        luaL_error(L, "can't add a %s to a closed window", type.c_str());
    }

    node->readProperties(L, index);
    node->createControl(L, parent, position);
    node->name = name;
    node->order = order;
    node->parent = parent;
    if (parent) parent->children.insert(parent->children.begin() + (std::ptrdiff_t)position, node);

    try {
        if (const char* event = node->activationEvent()) {
            std::weak_ptr<Node> weak = node;
            node->activatable()->onActivated = [weak, event] {
                std::shared_ptr<Node> node = weak.lock();
                if (!node || !node->alive || !luauHost) return;
                Luwow::Engine::StateLock lock(luauHost);
                fireSignal(luauHost->getMainState(), *node, event, 0);
            };

            // A handler in the descriptor is connected to the event's signal
            lua_getfield(L, index, event);
            if (lua_isfunction(L, -1)) {
                pushSignal(L, *node, event);
                Signals::connect(L, -1, -2);
                lua_pop(L, 2);
            } else if (!lua_isnil(L, -1)) {
                luaL_error(L, "a %s's %s must be a function", type.c_str(), event);
            }
            lua_pop(L, 1);
        }

        if (node->takesChildren()) {
            lua_getfield(L, index, "Children");
            if (!lua_isnil(L, -1)) buildChildren(L, -1, gui, *node);
            lua_pop(L, 1);
        }
    } catch (...) {
        destroyNode(L, *node);
        if (parent) {
            auto& siblings = parent->children;
            auto it = std::find(siblings.begin(), siblings.end(), node);
            if (it != siblings.end()) siblings.erase(it);
        }
        node->parent = nullptr;
        throw;
    }

    return node;
}

void registerNodes(lua_State* L, ILuauHost* host, IGuiModule* gui) {
    luauHost = host;
    guiModule = gui;
    Signals::registerSignals(L, host);

    lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, CacheKey);

    AllNodeTypes::registerHandles(L);
}

int callOnMainThread(lua_State* L, int (*function)(lua_State*)) {
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushcclosure(L, function, "gui", 1);
    lua_insert(L, 1);
    return callFunctionOnMainThread(L);
}

} // namespace Luwow::Gui
