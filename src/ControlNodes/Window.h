#pragma once

#include "Control.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A window, the root of a tree, with buttons, text native controls and a menu bar as its children
class Window : public Control {
public:
    static constexpr const char* TypeName = "Window";
    static const Properties<Window>& GetProperties();

    template <typename H>
    static std::vector<typename H::MethodDef> Methods() {
        return {
            { "show", &H::template invoke<&Window::show> },
            { "close", &H::template invoke<&Window::close> },
        };
    }

    // The native control of the window the control is in, nullptr once it's closed or when the control is detached
    static IWindow* containing(const Control& control);
    // Whether the control is in a window that's closed
    static bool isClosed(const Control& control);

    using Control::Control;
    const char* typeName() const override { return TypeName; }
    void pushHandle(lua_State* L) override;
    bool takesChildren() const override { return true; }
    bool acceptsChild(const Control& child) const override;
    std::string refuseChild(const Control& child) const override;
    void readProperties(lua_State* L, int index) override;
    void createNative(lua_State* L, Control* parent, size_t position) override;
    void destroyNative() override { native.reset(); }
    void sync(lua_State* L) override;

    WindowDescriptor properties;
    std::unique_ptr<IWindow> native;

private:
    static int show(lua_State* L, Window& window);
    static int close(lua_State* L, Window& window);
    // Gives the native control the bounds set, then reads back the ones it took
    static void applyBounds(Window& window);
    // Reads the window's bounds as they are on screen into its properties
    void readBounds();

    // Set while the window's own setBounds runs, whose changes the setter fires itself
    bool applyingBounds = false;
};

} // namespace Luwow::Gui::ControlNode
