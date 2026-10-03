#pragma once

#include "IGuiModule.h"
#include "Userdata.h"

namespace Luwow::Gui {

// Luau handles for the platform objects. They don't own the objects, whose lifetime is tied to the native window.
class LuauWindow : public Luwow::Engine::Userdata<LuauWindow> {
public:
    static constexpr const char* Name = "Window";

    explicit LuauWindow(IWindow* window) : window(window) {}

    // window:show()
    int show(lua_State* L);
    // window:close()
    int close(lua_State* L);

    IWindow* get() const { return window; }
    static void RegisterClass(lua_State* L);

private:
    IWindow* window;
};

class LuauButton : public Luwow::Engine::Userdata<LuauButton> {
public:
    static constexpr const char* Name = "Button";

    explicit LuauButton(IButton* button) : button(button) {}

    IButton* get() const { return button; }
    static void RegisterClass(lua_State* L);

private:
    IButton* button;
};

class LuauMenuBar : public Luwow::Engine::Userdata<LuauMenuBar> {
public:
    static constexpr const char* Name = "MenuBar";

    explicit LuauMenuBar(IMenuBar* menuBar) : menuBar(menuBar) {}

    IMenuBar* get() const { return menuBar; }
    static void RegisterClass(lua_State* L);

private:
    IMenuBar* menuBar;
};

// Registers every GUI userdata metatable on the given state.
void registerHandles(lua_State* L);

} // namespace Luwow::Gui
