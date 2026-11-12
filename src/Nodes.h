#pragma once

#include "IGuiModule.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct lua_State;

namespace Luwow::Gui {

enum class NodeKind {
    Window,
    Button,
    MenuBar,
    Menu,
    MenuItem,
};

// A descriptor made into a control at runtime, with its children as nodes too.
class Node {
public:
    Node(NodeKind kind, IGuiModule* module) : kind(kind), module(module) {}
    virtual ~Node() = default;

    const NodeKind kind;
    IGuiModule* const module;
    std::string name;
    // Where it goes among the siblings of a Children table, lower first
    double order = 0;
    Node* parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;
    bool alive = true;
    // Registry refs of the node's signals, by event name or "Changed:<property>"
    std::unordered_map<std::string, int> signals;

    // Destroys the node's control, its children's being already gone
    virtual void destroyControl() = 0;
};

class WindowNode : public Node {
public:
    explicit WindowNode(IGuiModule* module) : Node(NodeKind::Window, module) {}
    void destroyControl() override { control.reset(); }

    WindowDescriptor properties;
    std::unique_ptr<IWindow> control;
    bool applyingBounds = false;
};

class ButtonNode : public Node {
public:
    explicit ButtonNode(IGuiModule* module) : Node(NodeKind::Button, module) {}
    void destroyControl() override { control.reset(); }

    ButtonDescriptor properties;
    std::unique_ptr<IButton> control;
};

class MenuBarNode : public Node {
public:
    explicit MenuBarNode(IGuiModule* module) : Node(NodeKind::MenuBar, module) {}
    void destroyControl() override { control.reset(); }

    std::unique_ptr<IMenuBar> control;
};

class MenuNode : public Node {
public:
    explicit MenuNode(IGuiModule* module) : Node(NodeKind::Menu, module) {}
    void destroyControl() override { control.reset(); }

    MenuDescriptor properties;
    std::unique_ptr<IMenu> control;
};

class MenuItemNode : public Node {
public:
    explicit MenuItemNode(IGuiModule* module) : Node(NodeKind::MenuItem, module) {}
    void destroyControl() override { control.reset(); }

    MenuItemDescriptor properties;
    std::unique_ptr<IMenuItem> control;
};

// Registers the node userdata and the signals they use on the state
void registerNodes(lua_State* L, Luwow::Engine::ILuauHost* host, IGuiModule* gui);

// Runs function on the main thread with the call's arguments and returns its results. The function gets
// the running closure's first upvalue as its own. Another thread waits for it, its state lock released.
int callOnMainThread(lua_State* L, int (*function)(lua_State*));

// gui.createWindow(descriptor), and the Window node of its descriptor's tree. Everything else is added
// to the tree with node:add.
int createWindow(lua_State* L, IGuiModule* gui);
// gui.showFileDialog(descriptor, parent?), the chosen paths, nil when it's cancelled
int showFileDialog(lua_State* L, IGuiModule* gui);

} // namespace Luwow::Gui
