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

// A descriptor made into a control at runtime, with its children as controls too. A control has its native control while
// it's in a window's tree, and is detached, its properties kept for its native control, until it's added to one.
// Each kind of control is a ControlNode, in its own file, which implements the pure virtual methods and
// overrides the defaults it needs to.
class Control : public std::enable_shared_from_this<Control> {
public:
    explicit Control(IGuiModule* module) : module(module) {}
    // Its children that are still held become detached
    virtual ~Control() {
        for (const std::shared_ptr<Control>& child : children) child->parent = nullptr;
    }

    IGuiModule* const module;
    std::string name;
    // Where it goes among the siblings of a Children table, lower first
    double order = 0;
    Control* parent = nullptr;
    std::vector<std::shared_ptr<Control>> children;
    bool alive = true;
    // Registry refs of the control's signals, by event name or "Changed:<property>"
    std::unordered_map<std::string, int> signals;

    // The descriptor's Type, and the name of the handle's class
    virtual const char* typeName() const = 0;
    // Pushes a new handle of the control
    virtual void pushHandle(lua_State* L) = 0;

    // Whether the control has Children, and which kinds they can be. None by default.
    virtual bool takesChildren() const { return false; }
    virtual bool acceptsChild(const Control& child) const { (void)child; return false; }
    // Why child can't be added to the control, empty when it can
    virtual std::string refuseChild(const Control& child) const;
    // The event the control fires when the user activates its native control, and the native control. None by default.
    virtual const char* commandEvent() const { return nullptr; }
    virtual ICommandControl* commandControl() { return nullptr; }

    // Reads the properties of the descriptor table at index
    virtual void readProperties(lua_State* L, int index) = 0;
    // Makes the control's native control from its properties, at position among parent's children. The parent's
    // window is open.
    virtual void createNative(lua_State* L, Control* parent, size_t position) = 0;
    // Destroys the control's native control, its children's being already gone
    virtual void destroyNative() = 0;
    // Brings the properties up to date with what the user changed on screen, firing their Changed
    // signals. Nothing changes them by default.
    virtual void sync(lua_State* L) { (void)L; }
};

// The host the controls' state belongs to
Luwow::Engine::ILuauHost* controlHost();

// Whether className is a control's ClassName, and the ClassNames as "A, B or C"
bool isControlType(const std::string& className);
std::string controlTypeNames();

// The control's handle, the same userdata each time while one is held
void pushControl(lua_State* L, const std::shared_ptr<Control>& control);
// The control of a handle of any kind at index, nullptr for anything else
std::shared_ptr<Control> controlArg(lua_State* L, int index);
// Makes the descriptor table at index into a control with its children, under parent at position or detached
// without one. They get their native controls when they're in a window's tree. A control keyed in its parent's
// Children is named by the key.
std::shared_ptr<Control> buildControl(lua_State* L, int index, IGuiModule* gui, Control* parent, size_t position, const char* key = nullptr);
// Adds the detached control to parent at position, making its tree's native controls when parent is in a window
void attachControl(lua_State* L, const std::shared_ptr<Control>& control, Control& parent, size_t position);
// gui.createComponent(descriptor), the control of any descriptor Type with its Children
int createComponent(lua_State* L, IGuiModule* gui);
// Takes the child at index and its children away
void removeChild(lua_State* L, Control& parent, size_t index);
// Where a child goes or is: a 1-based position, the child itself, or for removal its name
size_t positionOf(lua_State* L, Control& parent, int index, bool inserting);
std::shared_ptr<Control> childNamed(Control& control, const char* name);

// The control's signal, made the first time it's asked for
void pushSignal(lua_State* L, Control& control, const std::string& key);
// Fires the control's signal with the nargs values on top of the stack, which are popped. Nothing fires for
// a signal nothing asked for.
void fireSignal(lua_State* L, Control& control, const std::string& key, int nargs);

// Calls the function at index 1 with the values above it on the main thread, returning its results
int callFunctionOnMainThread(lua_State* L);

// Registers the handles and the signals they use on the state
void registerControls(lua_State* L, Luwow::Engine::ILuauHost* host, IGuiModule* gui);

// Runs function on the main thread with the call's arguments and returns its results. The function gets
// the running closure's first upvalue as its own. Another thread waits for it, its state lock released.
int callOnMainThread(lua_State* L, int (*function)(lua_State*));

} // namespace Luwow::Gui
