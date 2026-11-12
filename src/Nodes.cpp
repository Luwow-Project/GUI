#include "Nodes.h"
#include "ILuauHost.h"
#include "SignalCore.h"
#include "Userdata.h"
#include "lua.h"
#include "lualib.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_set>

namespace Luwow::Gui {

using Luwow::Engine::ILuauHost;

namespace {

ILuauHost* luauHost = nullptr;
IGuiModule* guiModule = nullptr;

// Each node's userdata by the node's address, weak so a handle no script holds can be collected
const char* const CacheKey = "Luwow.Gui.Nodes";

// The windows made, kept for the program's life. Never destroyed: at exit the platform may already be gone.
std::vector<std::shared_ptr<Node>>& windows() {
    static auto* list = new std::vector<std::shared_ptr<Node>>();
    return *list;
}

const char* typeName(NodeKind kind) {
    switch (kind) {
        case NodeKind::Window: return "Window";
        case NodeKind::Button: return "Button";
        case NodeKind::MenuBar: return "MenuBar";
        case NodeKind::Menu: return "Menu";
        case NodeKind::MenuItem: return "MenuItem";
    }
    return "?";
}

std::optional<NodeKind> kindOf(const std::string& type) {
    if (type == "Window") return NodeKind::Window;
    if (type == "Button") return NodeKind::Button;
    if (type == "MenuBar") return NodeKind::MenuBar;
    if (type == "Menu") return NodeKind::Menu;
    if (type == "MenuItem") return NodeKind::MenuItem;
    return std::nullopt;
}

bool takesChildren(NodeKind kind) {
    return kind == NodeKind::Window || kind == NodeKind::MenuBar || kind == NodeKind::Menu;
}

bool acceptsChild(NodeKind parent, NodeKind child) {
    switch (parent) {
        case NodeKind::Window: return child == NodeKind::Button || child == NodeKind::MenuBar;
        case NodeKind::MenuBar: return child == NodeKind::Menu;
        case NodeKind::Menu: return child == NodeKind::MenuItem;
        default: return false;
    }
}

// The field older descriptors list children in, before Children
const char* legacyChildrenField(NodeKind kind) {
    if (kind == NodeKind::MenuBar) return "Menus";
    if (kind == NodeKind::Menu) return "Items";
    return nullptr;
}

// The event a kind fires when the user activates it
const char* eventOf(NodeKind kind) {
    if (kind == NodeKind::Button) return "OnPressed";
    if (kind == NodeKind::MenuItem) return "OnSelected";
    return nullptr;
}

// Each kind's properties and methods, which a child's name can't take, filled as the classes register
std::unordered_map<NodeKind, std::unordered_set<std::string>>& memberNames() {
    static std::unordered_map<NodeKind, std::unordered_set<std::string>> names;
    return names;
}

std::unordered_map<NodeKind, std::unordered_set<std::string>>& propertyNames() {
    static std::unordered_map<NodeKind, std::unordered_set<std::string>> names;
    return names;
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

// Fires the node's signal with the nargs values on top of the stack, which are popped. Nothing fires for
// a signal nothing asked for.
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

void firePropertyChanged(lua_State* L, Node& node, const char* property, int valueIndex) {
    lua_pushvalue(L, valueIndex);
    fireSignal(L, node, std::string("Changed:") + property, 1);
}

// Reads the window's bounds as they are on screen into its properties, firing Position and Size's signals
// for those that changed when fire is set
void syncWindowBounds(lua_State* L, WindowNode& node, bool fire) {
    if (!node.control || !node.control->isOpen()) return;
    WindowDescriptor& p = node.properties;
    int left = p.Left, top = p.Top, width = p.Width, height = p.Height;
    node.control->getBounds(left, top, width, height);
    bool moved = left != p.Left || top != p.Top;
    bool resized = width != p.Width || height != p.Height;
    p.Left = left;
    p.Top = top;
    p.Width = width;
    p.Height = height;
    if (!fire) return;
    if (moved) {
        pushPair(L, left, top);
        fireSignal(L, node, "Changed:Position", 1);
    }
    if (resized) {
        pushPair(L, width, height);
        fireSignal(L, node, "Changed:Size", 1);
    }
}

void releaseSignals(lua_State* L, Node& node) {
    for (auto& [key, ref] : node.signals) {
        lua_getref(L, ref);
        Signals::disconnectAll(L, -1);
        lua_pop(L, 1);
        lua_unref(L, ref);
    }
    node.signals.clear();
}

// The tree

WindowNode* windowOf(Node* node) {
    while (node && node->kind != NodeKind::Window) node = node->parent;
    return static_cast<WindowNode*>(node);
}

std::shared_ptr<Node> childNamed(Node& node, const char* name) {
    for (const std::shared_ptr<Node>& child : node.children) {
        if (child->name == name) return child;
    }
    return nullptr;
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

// Calls the function at index 1 with the values above it on the main thread, returning its results
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

void pushNode(lua_State* L, const std::shared_ptr<Node>& node);
std::shared_ptr<Node> nodeArg(lua_State* L, int index);
void pushDescription(lua_State* L, Node& node);
std::shared_ptr<Node> buildNode(lua_State* L, int index, IGuiModule* gui, Node* parent, size_t position, const char* key = nullptr);

// Where a child goes or is: a 1-based position, the child itself, or for removal its name
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
        luaL_error(L, "the %s has no child named '%s'", typeName(parent.kind), name);
    }
    if (std::shared_ptr<Node> node = nodeArg(L, index)) {
        for (size_t i = 0; i < count; ++i) {
            if (parent.children[i] == node) return i;
        }
        luaL_error(L, "the %s isn't a child of this %s", typeName(node->kind), typeName(parent.kind));
    }
    luaL_error(L, inserting ? "the position must be a number or a child node" : "expected a child node, its name or its position");
    return 0;
}

// Handles: one userdata class a kind, sharing the members every node has

template <typename Self, typename NodeT>
class NodeHandle : public Luwow::Engine::Userdata<Self> {
public:
    using MethodDef = typename Luwow::Engine::Userdata<Self>::MethodDef;
    using PropertyDef = typename Luwow::Engine::Userdata<Self>::PropertyDef;

    explicit NodeHandle(std::shared_ptr<Node> node) : node(std::move(node)) {}

    std::shared_ptr<Node> node;

    NodeT& self(lua_State* L) {
        if (!node->alive) luaL_error(L, "the %s was removed", Self::Name);
        return static_cast<NodeT&>(*node);
    }

    // node.Name, nil for an unnamed node
    int getName(lua_State* L) {
        if (node->name.empty()) lua_pushnil(L);
        else lua_pushlstring(L, node->name.data(), node->name.size());
        return 1;
    }

    // node.ClassName, the descriptor's Type
    int getClassName(lua_State* L) {
        lua_pushstring(L, Self::Name);
        return 1;
    }

    // node.Parent, nil for a window
    int getParent(lua_State* L) {
        Node& node = self(L);
        if (!node.parent) {
            lua_pushnil(L);
            return 1;
        }
        Node* parent = node.parent;
        std::shared_ptr<Node> shared = parent->parent ? childOf(*parent->parent, parent) : windowNamed(parent);
        if (shared) pushNode(L, shared);
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

    // node:bindToPropertyChanged(property), a signal fired with the new value when the property is set
    int bindToPropertyChanged(lua_State* L) {
        Node& node = self(L);
        const char* property = luaL_checkstring(L, 2);
        const auto& properties = propertyNames()[node.kind];
        if (properties.find(property) == properties.end()) {
            luaL_error(L, "%s has no property '%s'", Self::Name, property);
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
        size_t index = positionOf(L, node, 2, false);
        std::shared_ptr<Node> child = node.children[index];
        destroyNode(L, *child);
        child->parent = nullptr;
        node.children.erase(node.children.begin() + (std::ptrdiff_t)index);
        return 0;
    }

    // Registers the class with the members every node has, and makes node.ChildName find a child
    static void RegisterNode(lua_State* L, NodeKind kind, std::vector<MethodDef> methods, std::vector<PropertyDef> properties) {
        methods.push_back({ "getDescription", &NodeHandle::getDescription });
        methods.push_back({ "getChildren", &NodeHandle::getChildren });
        methods.push_back({ "bindToPropertyChanged", &NodeHandle::bindToPropertyChanged });
        if (takesChildren(kind)) {
            methods.push_back({ "add", &NodeHandle::add });
            methods.push_back({ "remove", &NodeHandle::remove });
        }
        properties.push_back({ "Name", &NodeHandle::getName, nullptr });
        properties.push_back({ "ClassName", &NodeHandle::getClassName, nullptr });
        properties.push_back({ "Parent", &NodeHandle::getParent, nullptr });

        auto& members = memberNames()[kind];
        auto& propertySet = propertyNames()[kind];
        const char* event = eventOf(kind);
        for (const MethodDef& method : methods) members.insert(method.name);
        for (const PropertyDef& property : properties) {
            members.insert(property.name);
            if (!event || std::string(property.name) != event) propertySet.insert(property.name);
        }

        Luwow::Engine::Userdata<Self>::Register(L, methods, properties);

        // The generated __index runs when the key isn't a child's name. Methods and setters touch the
        // controls, which other threads have the main thread run.
        luaL_getmetatable(L, Self::Name);
        lua_getfield(L, -1, "__index");
        lua_pushcclosure(L, IndexChildren, Self::Name, 1);
        lua_setfield(L, -2, "__index");
        for (const char* event : { "__namecall", "__newindex" }) {
            lua_getfield(L, -1, event);
            lua_pushcclosure(L, OnMainThread, event, 1);
            lua_setfield(L, -2, event);
        }
        lua_pop(L, 1);
    }

private:
    static int IndexChildren(lua_State* L) {
        Self* handle = Self::Check(L, 1);
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

    static std::shared_ptr<Node> childOf(Node& parent, Node* child) {
        for (const std::shared_ptr<Node>& candidate : parent.children) {
            if (candidate.get() == child) return candidate;
        }
        return nullptr;
    }

    static std::shared_ptr<Node> windowNamed(Node* window) {
        for (const std::shared_ptr<Node>& candidate : windows()) {
            if (candidate.get() == window) return candidate;
        }
        return nullptr;
    }
};

class WindowHandle : public NodeHandle<WindowHandle, WindowNode> {
public:
    static constexpr const char* Name = "Window";
    using NodeHandle::NodeHandle;

    // window:show()
    int show(lua_State* L) {
        self(L).control->show();
        return 0;
    }

    // window:close()
    int close(lua_State* L) {
        self(L).control->close();
        return 0;
    }

    int getTitle(lua_State* L) {
        lua_pushstring(L, self(L).properties.Title.c_str());
        return 1;
    }

    int setTitle(lua_State* L) {
        WindowNode& node = self(L);
        const char* title = luaL_checkstring(L, 3);
        if (node.properties.Title == title) return 0;
        node.properties.Title = title;
        node.control->setTitle(node.properties.Title);
        firePropertyChanged(L, node, "Title", 3);
        return 0;
    }

    // Size as {width, height} and Position as {x, y}, a new table each time
    template <int WindowDescriptor::*First, int WindowDescriptor::*Second>
    int getPair(lua_State* L) {
        const WindowDescriptor& p = self(L).properties;
        pushPair(L, p.*First, p.*Second);
        return 1;
    }

    // Starts from the bounds the window has now, so the other of Size and Position stays where the user
    // left it. The platform may adjust them, the signals get the bounds the window ends up with.
    template <int WindowDescriptor::*First, int WindowDescriptor::*Second>
    int setPair(lua_State* L) {
        WindowNode& node = self(L);
        int first = 0;
        int second = 0;
        if (!readPair(L, 3, first, second)) luaL_error(L, "%s must be a table of two numbers", lua_tostring(L, 2));
        syncWindowBounds(L, node, true);
        WindowDescriptor& p = node.properties;
        if (p.*First == first && p.*Second == second) return 0;
        WindowDescriptor before = p;
        p.*First = first;
        p.*Second = second;
        node.applyingBounds = true;
        node.control->setBounds(p.Left, p.Top, p.Width, p.Height);
        node.applyingBounds = false;
        syncWindowBounds(L, node, false);
        if (p.Left != before.Left || p.Top != before.Top) {
            pushPair(L, p.Left, p.Top);
            fireSignal(L, node, "Changed:Position", 1);
        }
        if (p.Width != before.Width || p.Height != before.Height) {
            pushPair(L, p.Width, p.Height);
            fireSignal(L, node, "Changed:Size", 1);
        }
        return 0;
    }

    static void RegisterClass(lua_State* L) {
        RegisterNode(L, NodeKind::Window, {
            { "show", &WindowHandle::show },
            { "close", &WindowHandle::close },
        }, {
            { "Title", &WindowHandle::getTitle, &WindowHandle::setTitle },
            { "Size", &WindowHandle::getPair<&WindowDescriptor::Width, &WindowDescriptor::Height>, &WindowHandle::setPair<&WindowDescriptor::Width, &WindowDescriptor::Height> },
            { "Position", &WindowHandle::getPair<&WindowDescriptor::Left, &WindowDescriptor::Top>, &WindowHandle::setPair<&WindowDescriptor::Left, &WindowDescriptor::Top> },
        });
    }
};

class ButtonHandle : public NodeHandle<ButtonHandle, ButtonNode> {
public:
    static constexpr const char* Name = "Button";
    using NodeHandle::NodeHandle;

    int getCaption(lua_State* L) {
        lua_pushstring(L, self(L).properties.Caption.c_str());
        return 1;
    }

    int setCaption(lua_State* L) {
        ButtonNode& node = self(L);
        const char* caption = luaL_checkstring(L, 3);
        if (node.properties.Caption == caption) return 0;
        node.properties.Caption = caption;
        node.control->setCaption(node.properties.Caption);
        firePropertyChanged(L, node, "Caption", 3);
        return 0;
    }

    // Size as {width, height} and Position as {x, y}, a new table each time
    template <int ButtonDescriptor::*First, int ButtonDescriptor::*Second>
    int getPair(lua_State* L) {
        const ButtonDescriptor& p = self(L).properties;
        pushPair(L, p.*First, p.*Second);
        return 1;
    }

    template <int ButtonDescriptor::*First, int ButtonDescriptor::*Second>
    int setPair(lua_State* L) {
        ButtonNode& node = self(L);
        int first = 0;
        int second = 0;
        if (!readPair(L, 3, first, second)) luaL_error(L, "%s must be a table of two numbers", lua_tostring(L, 2));
        ButtonDescriptor& p = node.properties;
        if (p.*First == first && p.*Second == second) return 0;
        p.*First = first;
        p.*Second = second;
        node.control->setBounds(p.Left, p.Top, p.Width, p.Height);
        pushPair(L, first, second);
        firePropertyChanged(L, node, lua_tostring(L, 2), lua_gettop(L));
        lua_pop(L, 1);
        return 0;
    }

    // button.OnPressed, a signal fired when it's pressed
    int getOnPressed(lua_State* L) {
        pushSignal(L, self(L), "OnPressed");
        return 1;
    }

    static void RegisterClass(lua_State* L) {
        RegisterNode(L, NodeKind::Button, {}, {
            { "Caption", &ButtonHandle::getCaption, &ButtonHandle::setCaption },
            { "Size", &ButtonHandle::getPair<&ButtonDescriptor::Width, &ButtonDescriptor::Height>, &ButtonHandle::setPair<&ButtonDescriptor::Width, &ButtonDescriptor::Height> },
            { "Position", &ButtonHandle::getPair<&ButtonDescriptor::Left, &ButtonDescriptor::Top>, &ButtonHandle::setPair<&ButtonDescriptor::Left, &ButtonDescriptor::Top> },
            { "OnPressed", &ButtonHandle::getOnPressed, nullptr },
        });
    }
};

class MenuBarHandle : public NodeHandle<MenuBarHandle, MenuBarNode> {
public:
    static constexpr const char* Name = "MenuBar";
    using NodeHandle::NodeHandle;

    static void RegisterClass(lua_State* L) {
        RegisterNode(L, NodeKind::MenuBar, {}, {});
    }
};

class MenuHandle : public NodeHandle<MenuHandle, MenuNode> {
public:
    static constexpr const char* Name = "Menu";
    using NodeHandle::NodeHandle;

    int getTitle(lua_State* L) {
        lua_pushstring(L, self(L).properties.Title.c_str());
        return 1;
    }

    int setTitle(lua_State* L) {
        MenuNode& node = self(L);
        const char* title = luaL_checkstring(L, 3);
        if (node.properties.Title == title) return 0;
        node.properties.Title = title;
        node.control->setTitle(node.properties.Title);
        firePropertyChanged(L, node, "Title", 3);
        return 0;
    }

    static void RegisterClass(lua_State* L) {
        RegisterNode(L, NodeKind::Menu, {}, {
            { "Title", &MenuHandle::getTitle, &MenuHandle::setTitle },
        });
    }
};

class MenuItemHandle : public NodeHandle<MenuItemHandle, MenuItemNode> {
public:
    static constexpr const char* Name = "MenuItem";
    using NodeHandle::NodeHandle;

    int getTitle(lua_State* L) {
        lua_pushstring(L, self(L).properties.Title.c_str());
        return 1;
    }

    int setTitle(lua_State* L) {
        MenuItemNode& node = self(L);
        const char* title = luaL_checkstring(L, 3);
        if (node.properties.Title == title) return 0;
        node.properties.Title = title;
        node.control->setTitle(node.properties.Title);
        firePropertyChanged(L, node, "Title", 3);
        return 0;
    }

    // item.OnSelected, a signal fired when it's selected
    int getOnSelected(lua_State* L) {
        pushSignal(L, self(L), "OnSelected");
        return 1;
    }

    static void RegisterClass(lua_State* L) {
        RegisterNode(L, NodeKind::MenuItem, {}, {
            { "Title", &MenuItemHandle::getTitle, &MenuItemHandle::setTitle },
            { "OnSelected", &MenuItemHandle::getOnSelected, nullptr },
        });
    }
};

void pushNode(lua_State* L, const std::shared_ptr<Node>& node) {
    lua_getfield(L, LUA_REGISTRYINDEX, CacheKey);
    lua_pushlightuserdata(L, node.get());
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);

    switch (node->kind) {
        case NodeKind::Window: WindowHandle::Push(L, node); break;
        case NodeKind::Button: ButtonHandle::Push(L, node); break;
        case NodeKind::MenuBar: MenuBarHandle::Push(L, node); break;
        case NodeKind::Menu: MenuHandle::Push(L, node); break;
        case NodeKind::MenuItem: MenuItemHandle::Push(L, node); break;
    }

    lua_pushlightuserdata(L, node.get());
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    lua_remove(L, -2);
}

// The node of a handle of any kind at index, nullptr for anything else
std::shared_ptr<Node> nodeArg(lua_State* L, int index) {
    if (!lua_isuserdata(L, index) || !lua_getmetatable(L, index)) return nullptr;
    void* data = lua_touserdata(L, index);
    auto is = [L](const char* name) {
        luaL_getmetatable(L, name);
        bool same = lua_rawequal(L, -1, -2);
        lua_pop(L, 1);
        return same;
    };

    std::shared_ptr<Node> node;
    if (is(WindowHandle::Name)) node = static_cast<WindowHandle*>(data)->node;
    else if (is(ButtonHandle::Name)) node = static_cast<ButtonHandle*>(data)->node;
    else if (is(MenuBarHandle::Name)) node = static_cast<MenuBarHandle*>(data)->node;
    else if (is(MenuHandle::Name)) node = static_cast<MenuHandle*>(data)->node;
    else if (is(MenuItemHandle::Name)) node = static_cast<MenuItemHandle*>(data)->node;
    lua_pop(L, 1);
    return node;
}

void pushDescription(lua_State* L, Node& node) {
    lua_createtable(L, 0, 8);
    lua_pushstring(L, typeName(node.kind));
    lua_setfield(L, -2, "Type");
    if (!node.name.empty()) {
        lua_pushlstring(L, node.name.data(), node.name.size());
        lua_setfield(L, -2, "Name");
    }
    if (node.order != 0) {
        lua_pushnumber(L, node.order);
        lua_setfield(L, -2, "Order");
    }

    auto setString = [L](const char* field, const std::string& value) {
        lua_pushlstring(L, value.data(), value.size());
        lua_setfield(L, -2, field);
    };
    auto setPair = [L](const char* field, int first, int second) {
        pushPair(L, first, second);
        lua_setfield(L, -2, field);
    };

    switch (node.kind) {
        case NodeKind::Window: {
            const WindowDescriptor& p = static_cast<WindowNode&>(node).properties;
            setString("Title", p.Title);
            setPair("Size", p.Width, p.Height);
            setPair("Position", p.Left, p.Top);
            break;
        }
        case NodeKind::Button: {
            const ButtonDescriptor& p = static_cast<ButtonNode&>(node).properties;
            setString("Caption", p.Caption);
            setPair("Size", p.Width, p.Height);
            setPair("Position", p.Left, p.Top);
            break;
        }
        case NodeKind::Menu:
            setString("Title", static_cast<MenuNode&>(node).properties.Title);
            break;
        case NodeKind::MenuItem:
            setString("Title", static_cast<MenuItemNode&>(node).properties.Title);
            break;
        case NodeKind::MenuBar:
            break;
    }

    if (takesChildren(node.kind)) {
        lua_createtable(L, (int)node.children.size(), 0);
        for (size_t i = 0; i < node.children.size(); ++i) {
            pushDescription(L, *node.children[i]);
            lua_rawseti(L, -2, (int)i + 1);
        }
        lua_setfield(L, -2, "Children");
    }
}

// Builds a node's Children table at index: an array of descriptors with optional Names, and descriptors
// keyed by their names. They're ordered by Order, then the array's in its order, then the keyed by name.
void buildChildren(lua_State* L, int index, IGuiModule* gui, Node& node, const char* type) {
    index = lua_absindex(L, index);
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

// Makes the descriptor table at index into a node with its control, under parent at position, then its
// children. A node keyed in its parent's Children is named by the key. A node that fails partway is
// taken away again.
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
    std::optional<NodeKind> found = kindOf(type);
    if (!found) {
        luaL_error(L, "unknown descriptor Type '%s', expected Window, Button, MenuBar, Menu or MenuItem", type.c_str());
    }
    NodeKind kind = *found;

    if (!parent && kind != NodeKind::Window) {
        luaL_error(L, "a %s needs a parent", type.c_str());
    }
    if (parent && !acceptsChild(parent->kind, kind)) {
        luaL_error(L, "a %s can't be a child of a %s", type.c_str(), typeName(parent->kind));
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
        const auto& members = memberNames()[parent->kind];
        if (members.find(name) != members.end()) {
            luaL_error(L, "a child can't be named '%s', a member of %s", name.c_str(), typeName(parent->kind));
        }
        if (childNamed(*parent, name.c_str())) {
            luaL_error(L, "the %s already has a child named '%s'", typeName(parent->kind), name.c_str());
        }
    }

    if (!takesChildren(kind)) {
        lua_getfield(L, index, "Children");
        bool hasChildren = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (hasChildren) luaL_error(L, "%s descriptors don't take Children", type.c_str());
    }

    WindowNode* window = parent ? windowOf(parent) : nullptr;
    if (parent && (!window || !window->control || !window->control->isOpen())) {
        luaL_error(L, "can't add a %s to a closed window", type.c_str());
    }

    std::shared_ptr<Node> node;
    IActivatable* activatable = nullptr;
    switch (kind) {
        case NodeKind::Window: {
            auto created = std::make_shared<WindowNode>(gui);
            created->properties = readWindowDescriptor(L, index);
            created->control.reset(gui->createWindow(created->properties));
            node = created;
            break;
        }
        case NodeKind::Button: {
            auto created = std::make_shared<ButtonNode>(gui);
            created->properties = readButtonDescriptor(L, index);
            created->control.reset(gui->createButton(created->properties, window->control.get()));
            activatable = created->control.get();
            node = created;
            break;
        }
        case NodeKind::MenuBar: {
            for (const std::shared_ptr<Node>& sibling : parent->children) {
                if (sibling->kind == NodeKind::MenuBar) luaL_error(L, "a Window has only one MenuBar");
            }
            auto created = std::make_shared<MenuBarNode>(gui);
            created->control.reset(gui->createMenuBar(window->control.get()));
            node = created;
            break;
        }
        case NodeKind::Menu: {
            auto created = std::make_shared<MenuNode>(gui);
            created->properties = readMenuDescriptor(L, index);
            created->control.reset(gui->createMenu(created->properties));
            static_cast<MenuBarNode*>(parent)->control->insertMenu(position, created->control.get());
            node = created;
            break;
        }
        case NodeKind::MenuItem: {
            auto created = std::make_shared<MenuItemNode>(gui);
            created->properties = readMenuItemDescriptor(L, index);
            created->control.reset(gui->createMenuItem(created->properties, window->control.get()));
            static_cast<MenuNode*>(parent)->control->insertItem(position, created->control.get());
            activatable = created->control.get();
            node = created;
            break;
        }
    }

    node->name = name;
    node->order = order;
    node->parent = parent;

    if (kind == NodeKind::Window) {
        WindowNode& windowNode = static_cast<WindowNode&>(*node);
        syncWindowBounds(L, windowNode, false);
        std::weak_ptr<Node> weak = node;
        windowNode.control->onBoundsChanged = [weak] {
            std::shared_ptr<Node> node = weak.lock();
            if (!node || !node->alive || !luauHost) return;
            WindowNode& windowNode = static_cast<WindowNode&>(*node);
            if (windowNode.applyingBounds) return;
            Luwow::Engine::StateLock lock(luauHost);
            syncWindowBounds(luauHost->getMainState(), windowNode, true);
        };
    }
    if (parent) parent->children.insert(parent->children.begin() + (std::ptrdiff_t)position, node);

    try {
        if (const char* event = eventOf(kind)) {
            std::weak_ptr<Node> weak = node;
            activatable->onActivated = [weak, event] {
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

        if (takesChildren(kind)) {
            lua_getfield(L, index, "Children");
            const char* legacy = legacyChildrenField(kind);
            if (lua_isnil(L, -1) && legacy) {
                lua_pop(L, 1);
                lua_getfield(L, index, legacy);
            }
            if (!lua_isnil(L, -1)) buildChildren(L, -1, gui, *node, type.c_str());
            lua_pop(L, 1);
        }
    } catch (...) {
        destroyNode(L, *node);
        if (parent) {
            auto& siblings = parent->children;
            for (auto it = siblings.begin(); it != siblings.end(); ++it) {
                if (*it == node) {
                    siblings.erase(it);
                    break;
                }
            }
        }
        node->parent = nullptr;
        throw;
    }

    return node;
}

// The Type of the descriptor at index must be type
void requireType(lua_State* L, int index, const char* type) {
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getfield(L, index, "Type");
    const char* actual = lua_tostring(L, -1);
    if (!actual || std::string(actual) != type) {
        luaL_error(L, "%s descriptor must have a Type of %s", type, type);
    }
    lua_pop(L, 1);
}

} // namespace

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

    WindowHandle::RegisterClass(L);
    ButtonHandle::RegisterClass(L);
    MenuBarHandle::RegisterClass(L);
    MenuHandle::RegisterClass(L);
    MenuItemHandle::RegisterClass(L);
}

int createWindow(lua_State* L, IGuiModule* gui) {
    requireType(L, 1, "Window");
    std::shared_ptr<Node> window = buildNode(L, 1, gui, nullptr, 0);
    windows().push_back(window);
    pushNode(L, window);
    return 1;
}

int showFileDialog(lua_State* L, IGuiModule* gui) {
    FileDialogDescriptor descriptor = getFileDialogDescriptor(L, 1);
    IWindow* parent = lua_isnoneornil(L, 2) ? nullptr : WindowHandle::Check(L, 2)->self(L).control.get();
    std::vector<std::string> paths = gui->showFileDialog(descriptor, parent);
    if (paths.empty()) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, (int)paths.size(), 0);
    for (size_t i = 0; i < paths.size(); ++i) {
        lua_pushlstring(L, paths[i].data(), paths[i].size());
        lua_rawseti(L, -2, (int)i + 1);
    }
    return 1;
}

int callOnMainThread(lua_State* L, int (*function)(lua_State*)) {
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushcclosure(L, function, "gui", 1);
    lua_insert(L, 1);
    return callFunctionOnMainThread(L);
}

} // namespace Luwow::Gui
