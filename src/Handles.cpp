#include "Handles.h"
#include "lualib.h"

namespace Luwow::Gui {

int LuauWindow::show(lua_State* L) {
    window->show();
    return 0;
}

int LuauWindow::close(lua_State* L) {
    window->close();
    return 0;
}

int LuauWindow::getNativeHandle(lua_State* L) {
    lua_pushlightuserdata(L, window->getNativeHandle());
    return 1;
}

void LuauWindow::RegisterClass(lua_State* L) {
    Register(L, {
        { "show",  &LuauWindow::show },
        { "close", &LuauWindow::close },
        { "getNativeHandle", &LuauWindow::getNativeHandle }
    }, {});
}

void LuauButton::RegisterClass(lua_State* L) {
    Register(L, {}, {});
}

void LuauMenuBar::RegisterClass(lua_State* L) {
    Register(L, {}, {});
}

void registerHandles(lua_State* L) {
    LuauWindow::RegisterClass(L);
    LuauButton::RegisterClass(L);
    LuauMenuBar::RegisterClass(L);
}

} // namespace Luwow::Gui
