#include "Descriptors.h"
#include "lua.h"
#include "lualib.h"

namespace Luwow::Gui {

// Helper function to get the WindowDescriptor from the table.
// This file now also supports menu descriptors for Windows.
WindowDescriptor getWindowDescriptor(lua_State* L) {
    WindowDescriptor windowDescriptor;
    luaL_checktype(L, 1, LUA_TTABLE);
    
    lua_getfield(L, 1, "Type");
    windowDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (windowDescriptor.Type != "Window") {
        luaL_error(L, "Window descriptor must have a Type of Window");
    }
    
    lua_getfield(L, 1, "Title");
    windowDescriptor.Title = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Width");
    windowDescriptor.Width = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Height");
    windowDescriptor.Height = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Left");
    windowDescriptor.Left = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Top");
    windowDescriptor.Top = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    return windowDescriptor;
}

ButtonDescriptor getButtonDescriptor(lua_State* L) {
    ButtonDescriptor buttonDescriptor;
    luaL_checktype(L, 1, LUA_TTABLE);
    
    lua_getfield(L, 1, "Type");
    buttonDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (buttonDescriptor.Type != "Button") {
        luaL_error(L, "Button descriptor must have a Type of Button");
    }
    
    lua_getfield(L, 1, "Caption");
    buttonDescriptor.Caption = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // Store the Lua state
    buttonDescriptor.L = L;

    // Store the Lua function
    lua_getfield(L, 1, "OnPressed");
    luaL_checktype(L, -1, LUA_TFUNCTION);
    buttonDescriptor.OnPressedRef = lua_ref(L, -1);
    lua_pop(L, 1);
        
    lua_getfield(L, 1, "Left");
    buttonDescriptor.Left = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Top");
    buttonDescriptor.Top = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Width");
    buttonDescriptor.Width = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "Height");
    buttonDescriptor.Height = luaL_optnumber(L, -1, 0);
    lua_pop(L, 1);
    
    return buttonDescriptor;
}

TextBoxDescriptor getTextBoxDescriptor(lua_State* L) {
    TextBoxDescriptor textBoxDescriptor;
    luaL_checktype(L, 1, LUA_TTABLE);
    
    lua_getfield(L, 1, "Type");
    textBoxDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (textBoxDescriptor.Type != "TextBox") {
        luaL_error(L, "TextBox descriptor must have a Type of TextBox");
    }
    
    lua_getfield(L, 1, "Text");
    textBoxDescriptor.Text = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, 1, "MultiLineMode");
    textBoxDescriptor.MultiLineMode = luaL_checkboolean(L, -1);
    lua_pop(L, 1);
    
    return textBoxDescriptor;
}

MenuItemDescriptor getMenuItemDescriptor(lua_State* L) {
    MenuItemDescriptor itemDescriptor;
    luaL_checktype(L, -1, LUA_TTABLE);

    lua_getfield(L, -1, "Type");
    itemDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (itemDescriptor.Type != "MenuItem") {
        luaL_error(L, "Menu item descriptor must have a Type of MenuItem");
    }

    lua_getfield(L, -1, "Title");
    itemDescriptor.Title = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    itemDescriptor.L = L;
    lua_getfield(L, -1, "OnSelected");
    luaL_checktype(L, -1, LUA_TFUNCTION);
    itemDescriptor.OnSelectedRef = lua_ref(L, -1);
    lua_pop(L, 1);

    return itemDescriptor;
}

MenuDescriptor getMenuDescriptor(lua_State* L) {
    MenuDescriptor menuDescriptor;
    luaL_checktype(L, -1, LUA_TTABLE);

    lua_getfield(L, -1, "Type");
    menuDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (menuDescriptor.Type != "Menu") {
        luaL_error(L, "Menu descriptor must have a Type of Menu");
    }

    lua_getfield(L, -1, "Title");
    menuDescriptor.Title = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "Items");
    luaL_checktype(L, -1, LUA_TTABLE);
    int items = (int)lua_objlen(L, -1);
    for (int i = 1; i <= items; ++i) {
        lua_rawgeti(L, -1, i);
        menuDescriptor.Items.push_back(getMenuItemDescriptor(L));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    return menuDescriptor;
}

MenuBarDescriptor getMenuBarDescriptor(lua_State* L) {
    MenuBarDescriptor menuBarDescriptor;
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "Type");
    menuBarDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (menuBarDescriptor.Type != "MenuBar") {
        luaL_error(L, "Menu bar descriptor must have a Type of MenuBar");
    }

    lua_getfield(L, 1, "Menus");
    luaL_checktype(L, -1, LUA_TTABLE);
    int menus = (int)lua_objlen(L, -1);
    for (int i = 1; i <= menus; ++i) {
        lua_rawgeti(L, -1, i);
        menuBarDescriptor.Menus.push_back(getMenuDescriptor(L));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    return menuBarDescriptor;
}

} // namespace Luwow::Gui