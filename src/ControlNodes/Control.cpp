#include "Control.h"
#include "Window.h"
#include "Button.h"
#include "TextLabel.h"
#include "TextBox.h"
#include "TextArea.h"
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

// Each control's userdata by the control's address, weak so a handle no script holds can be collected
const char* const CacheKey = "Luwow.Gui.Controls";

// In the order they're registered
// The ControlNodes, and what's done with each of them in turn
template <typename... ControlNodes>
struct ControlTypes {
    // A control of the descriptor Type, without its native control yet, nullptr for an unknown Type
    static std::shared_ptr<Control> make(const std::string& type, IGuiModule* gui) {
        std::shared_ptr<Control> control;
        ((control = !control && type == ControlNodes::TypeName ? std::make_shared<ControlNodes>(gui) : control), ...);
        return control;
    }

    static std::shared_ptr<Control> fromHandle(lua_State* L, int index) {
        std::shared_ptr<Control> control;
        ((control = control ? control : Handle<ControlNodes>::ControlAt(L, index)), ...);
        return control;
    }

    static bool has(const std::string& type) {
        return ((type == ControlNodes::TypeName) || ...);
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

using AllControlTypes = ControlTypes<
    ControlNode::Window,
    ControlNode::Button,
    ControlNode::TextLabel,
    ControlNode::TextBox,
    ControlNode::TextArea,
    ControlNode::MenuBar,
    ControlNode::Menu,
    ControlNode::MenuItem
>;

void releaseSignals(lua_State* L, Control& control) {
    for (auto& [key, ref] : control.signals) {
        lua_getref(L, ref);
        Signals::disconnectAll(L, -1);
        lua_pop(L, 1);
        lua_unref(L, ref);
    }
    control.signals.clear();
}

// Takes the control and its children off the screen, deepest first, and ends their handles
void destroyTree(lua_State* L, Control& control) {
    for (auto it = control.children.rbegin(); it != control.children.rend(); ++it) {
        destroyTree(L, **it);
        (*it)->parent = nullptr;
    }
    control.children.clear();
    control.destroyNative();
    releaseSignals(L, control);
    control.alive = false;
}

std::shared_ptr<Control> readControlTree(lua_State* L, int index, IGuiModule* gui, Control* parent, size_t position, const char* key = nullptr);

// Raises why child can't go under parent: a kind parent doesn't take, a name a sibling has, or a closed
// window
void checkCanAdd(lua_State* L, Control& parent, const Control& child, const std::string& name) {
    std::string refusal = parent.refuseChild(child);
    if (!refusal.empty()) luaL_error(L, "%s", refusal.c_str());
    if (!name.empty() && childNamed(parent, name.c_str())) {
        luaL_error(L, "the %s already has a child named '%s'", parent.typeName(), name.c_str());
    }
    if (ControlNode::Window::isClosed(parent)) {
        luaL_error(L, "can't add a %s to a closed window", child.typeName());
    }
}

// Makes the native controls of the control's tree, the control's at position among its parent's children
void createNatives(lua_State* L, Control& control, size_t position) {
    control.createNative(L, control.parent, position);
    if (const char* event = control.commandEvent()) {
        std::weak_ptr<Control> weak = control.shared_from_this();
        control.commandControl()->onCommand = [weak, event] {
            std::shared_ptr<Control> control = weak.lock();
            if (!control || !control->alive || !luauHost) return;
            Luwow::Engine::StateLock lock(luauHost);
            fireSignal(luauHost->getMainState(), *control, event, 0);
        };
    }
    for (size_t i = 0; i < control.children.size(); ++i) {
        createNatives(L, *control.children[i], i);
    }
}

// Takes a control that failed partway away again
void discard(lua_State* L, const std::shared_ptr<Control>& control) {
    destroyTree(L, *control);
    if (Control* parent = control->parent) {
        auto& siblings = parent->children;
        auto it = std::find(siblings.begin(), siblings.end(), control);
        if (it != siblings.end()) siblings.erase(it);
    }
    control->parent = nullptr;
}

// Makes the native controls of a window's tree, or of a tree just put in an open window
void createNativesInWindow(lua_State* L, const std::shared_ptr<Control>& control, size_t position) {
    bool window = dynamic_cast<ControlNode::Window*>(control.get()) != nullptr;
    if (!window && !(control->parent && ControlNode::Window::containing(*control->parent))) return;
    try {
        createNatives(L, *control, position);
    } catch (...) {
        discard(L, control);
        throw;
    }
}

// Builds a control's Children table at index: an array of descriptors with optional Names, and descriptors
// keyed by their names. They're ordered by Order, then the array's in its order, then the keyed by name.
void readChildren(lua_State* L, int index, IGuiModule* gui, Control& control) {
    index = lua_absindex(L, index);
    const char* type = control.typeName();
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
            readControlTree(L, -1, gui, &control, control.children.size());
        } else {
            lua_pushlstring(L, entry.key.data(), entry.key.size());
            lua_rawget(L, index);
            readControlTree(L, -1, gui, &control, control.children.size(), entry.key.c_str());
        }
        lua_pop(L, 1);
    }
}

// The control of a descriptor's Type, without its native control yet
std::shared_ptr<Control> makeControl(lua_State* L, const std::string& type, IGuiModule* gui) {
    if (std::shared_ptr<Control> control = AllControlTypes::make(type, gui)) return control;
    luaL_error(L, "unknown descriptor Type '%s', expected %s", type.c_str(), AllControlTypes::names().c_str());
    return nullptr;
}

} // namespace

ILuauHost* controlHost() {
    return luauHost;
}

// Signals

void pushSignal(lua_State* L, Control& control, const std::string& key) {
    auto it = control.signals.find(key);
    if (it != control.signals.end()) {
        lua_getref(L, it->second);
        return;
    }
    Signals::pushSignal(L);
    control.signals[key] = lua_ref(L, -1);
}

void fireSignal(lua_State* L, Control& control, const std::string& key, int nargs) {
    auto it = control.signals.find(key);
    if (it == control.signals.end()) {
        lua_pop(L, nargs);
        return;
    }
    lua_getref(L, it->second);
    lua_insert(L, -(nargs + 1));
    Signals::fireSignal(L, -(nargs + 1), nargs);
    lua_pop(L, 1);
}

bool isControlType(const std::string& className) {
    return AllControlTypes::has(className);
}

std::string controlTypeNames() {
    return AllControlTypes::names();
}

// The tree

std::shared_ptr<Control> childNamed(Control& control, const char* name) {
    for (const std::shared_ptr<Control>& child : control.children) {
        if (child->name == name) return child;
    }
    return nullptr;
}

size_t positionOf(lua_State* L, Control& parent, int index, bool inserting) {
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
    if (std::shared_ptr<Control> control = controlArg(L, index)) {
        for (size_t i = 0; i < count; ++i) {
            if (parent.children[i] == control) return i;
        }
        luaL_error(L, "the %s isn't a child of this %s", control->typeName(), parent.typeName());
    }
    luaL_error(L, inserting ? "the position must be a number or a child control" : "expected a child control, its name or its position");
    return 0;
}

void removeChild(lua_State* L, Control& parent, size_t index) {
    std::shared_ptr<Control> child = parent.children[index];
    destroyTree(L, *child);
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

void pushControl(lua_State* L, const std::shared_ptr<Control>& control) {
    lua_getfield(L, LUA_REGISTRYINDEX, CacheKey);
    lua_pushlightuserdata(L, control.get());
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);

    control->pushHandle(L);

    lua_pushlightuserdata(L, control.get());
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    lua_remove(L, -2);
}

std::shared_ptr<Control> controlArg(lua_State* L, int index) {
    return AllControlTypes::fromHandle(L, index);
}

std::string Control::refuseChild(const Control& child) const {
    if (acceptsChild(child)) return {};
    return std::string("a ") + child.typeName() + " can't be a child of a " + typeName();
}

namespace {

// The control of the descriptor at index with its Children, without native controls. A control that fails partway is
// taken away again.
std::shared_ptr<Control> readControlTree(lua_State* L, int index, IGuiModule* gui, Control* parent, size_t position, const char* key) {
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
    std::shared_ptr<Control> control = makeControl(L, type, gui);

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
    if (parent) checkCanAdd(L, *parent, *control, name);

    if (!control->takesChildren()) {
        lua_getfield(L, index, "Children");
        bool hasChildren = !lua_isnil(L, -1);
        lua_pop(L, 1);
        if (hasChildren) luaL_error(L, "%s descriptors don't take Children", type.c_str());
    }

    control->readProperties(L, index);
    control->name = name;
    control->order = order;
    control->parent = parent;
    if (parent) parent->children.insert(parent->children.begin() + (std::ptrdiff_t)position, control);

    try {
        if (const char* event = control->commandEvent()) {
            // A handler in the descriptor is connected to the event's signal
            lua_getfield(L, index, event);
            if (lua_isfunction(L, -1)) {
                pushSignal(L, *control, event);
                Signals::connect(L, -1, -2);
                lua_pop(L, 2);
            } else if (!lua_isnil(L, -1)) {
                luaL_error(L, "a %s's %s must be a function", type.c_str(), event);
            }
            lua_pop(L, 1);
        }

        if (control->takesChildren()) {
            lua_getfield(L, index, "Children");
            if (!lua_isnil(L, -1)) readChildren(L, -1, gui, *control);
            lua_pop(L, 1);
        }
    } catch (...) {
        discard(L, control);
        throw;
    }

    return control;
}

} // namespace

std::shared_ptr<Control> buildControl(lua_State* L, int index, IGuiModule* gui, Control* parent, size_t position, const char* key) {
    std::shared_ptr<Control> control = readControlTree(L, index, gui, parent, position, key);
    createNativesInWindow(L, control, position);
    return control;
}

void attachControl(lua_State* L, const std::shared_ptr<Control>& control, Control& parent, size_t position) {
    if (control->parent) luaL_error(L, "the %s already has a parent", control->typeName());
    for (Control* ancestor = &parent; ancestor; ancestor = ancestor->parent) {
        if (ancestor == control.get()) luaL_error(L, "can't add a %s under itself", control->typeName());
    }
    checkCanAdd(L, parent, *control, control->name);
    control->parent = &parent;
    parent.children.insert(parent.children.begin() + (std::ptrdiff_t)position, control);
    createNativesInWindow(L, control, position);
}

int createComponent(lua_State* L, IGuiModule* gui) {
    luaL_checktype(L, 1, LUA_TTABLE);
    pushControl(L, buildControl(L, 1, gui, nullptr, 0));
    return 1;
}

void registerControls(lua_State* L, ILuauHost* host, IGuiModule* gui) {
    luauHost = host;
    guiModule = gui;
    Signals::registerSignals(L, host);

    lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, CacheKey);

    AllControlTypes::registerHandles(L);
}

int callOnMainThread(lua_State* L, int (*function)(lua_State*)) {
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushcclosure(L, function, "gui", 1);
    lua_insert(L, 1);
    return callFunctionOnMainThread(L);
}

} // namespace Luwow::Gui
