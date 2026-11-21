#pragma once

#include "IGuiModule.h"
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct lua_State;

namespace Luwow::Gui {

// A descriptor made into a node at runtime, with its children as nodes too. A node has its control while
// it's in a window's tree, and is detached, its properties kept for its control, until it's added to one.
// Each kind of node is a ControlNode, in its own file, which implements the pure virtual methods and
// overrides the defaults it needs to.
class Node : public std::enable_shared_from_this<Node> {
public:
    explicit Node(IGuiModule* module) : module(module) {}
    // Its children that are still held become detached
    virtual ~Node() {
        for (const std::shared_ptr<Node>& child : children) child->parent = nullptr;
    }

    IGuiModule* const module;
    std::string name;
    // Where it goes among the siblings of a Children table, lower first
    double order = 0;
    Node* parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;
    bool alive = true;
    // Registry refs of the node's signals, by event name or "Changed:<property>"
    std::unordered_map<std::string, int> signals;

    // The descriptor's Type, and the name of the handle's class
    virtual const char* typeName() const = 0;
    // Pushes a new handle of the node
    virtual void pushHandle(lua_State* L) = 0;

    // Whether the node has Children, and which kinds they can be. None by default.
    virtual bool takesChildren() const { return false; }
    virtual bool acceptsChild(const Node& child) const { (void)child; return false; }
    // Why child can't be added to the node, empty when it can
    virtual std::string refuseChild(const Node& child) const;
    // The event the node fires when the user activates its control, and the control. None by default.
    virtual const char* activationEvent() const { return nullptr; }
    virtual IActivatable* activatable() { return nullptr; }

    // Reads the properties of the descriptor table at index
    virtual void readProperties(lua_State* L, int index) = 0;
    // Makes the node's control from its properties, at position among parent's children. The parent's
    // window is open.
    virtual void createControl(lua_State* L, Node* parent, size_t position) = 0;
    // Destroys the node's control, its children's being already gone
    virtual void destroyControl() = 0;
    // Brings the properties up to date with what the user changed on screen, firing their Changed
    // signals. Nothing changes them by default.
    virtual void sync(lua_State* L) { (void)L; }
};

// The host the nodes' state belongs to
Luwow::Engine::ILuauHost* nodeHost();

// Whether className is a node's ClassName, and the ClassNames as "A, B or C"
bool isNodeType(const std::string& className);
std::string nodeTypeNames();

// The node's handle, the same userdata each time while one is held
void pushNode(lua_State* L, const std::shared_ptr<Node>& node);
// The node of a handle of any kind at index, nullptr for anything else
std::shared_ptr<Node> nodeArg(lua_State* L, int index);
// Makes the descriptor table at index into a node with its children, under parent at position or detached
// without one. They get their controls when they're in a window's tree. A node keyed in its parent's
// Children is named by the key.
std::shared_ptr<Node> buildNode(lua_State* L, int index, IGuiModule* gui, Node* parent, size_t position, const char* key = nullptr);
// Adds the detached node to parent at position, making its tree's controls when parent is in a window
void attachNode(lua_State* L, const std::shared_ptr<Node>& node, Node& parent, size_t position);
// gui.createComponent(descriptor), the node of any descriptor Type with its Children
int createComponent(lua_State* L, IGuiModule* gui);
// Takes the child at index and its children away
void removeChild(lua_State* L, Node& parent, size_t index);
// Where a child goes or is: a 1-based position, the child itself, or for removal its name
size_t positionOf(lua_State* L, Node& parent, int index, bool inserting);
std::shared_ptr<Node> childNamed(Node& node, const char* name);

// The node's signal, made the first time it's asked for
void pushSignal(lua_State* L, Node& node, const std::string& key);
// Fires the node's signal with the nargs values on top of the stack, which are popped. Nothing fires for
// a signal nothing asked for.
void fireSignal(lua_State* L, Node& node, const std::string& key, int nargs);

// Calls the function at index 1 with the values above it on the main thread, returning its results
int callFunctionOnMainThread(lua_State* L);

// Registers the handles and the signals they use on the state
void registerNodes(lua_State* L, Luwow::Engine::ILuauHost* host, IGuiModule* gui);

// Runs function on the main thread with the call's arguments and returns its results. The function gets
// the running closure's first upvalue as its own. Another thread waits for it, its state lock released.
int callOnMainThread(lua_State* L, int (*function)(lua_State*));

} // namespace Luwow::Gui
