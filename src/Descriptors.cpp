#include "Descriptors.h"
#include "lua.h"
#include "lualib.h"
#include <cstring>

namespace Luwow::Gui {

bool readPair(lua_State* L, int index, int& first, int& second) {
    if (!lua_istable(L, index) || lua_objlen(L, index) != 2) return false;
    lua_rawgeti(L, index, 1);
    lua_rawgeti(L, index < 0 ? index - 1 : index, 2);
    bool numbers = lua_isnumber(L, -2) && lua_isnumber(L, -1);
    if (numbers) {
        first = (int)lua_tonumber(L, -2);
        second = (int)lua_tonumber(L, -1);
    }
    lua_pop(L, 2);
    return numbers;
}

void pushPair(lua_State* L, int first, int second) {
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, first);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, second);
    lua_rawseti(L, -2, 2);
}

static FileDialogFilter getFileDialogFilter(lua_State* L) {
    FileDialogFilter filter;
    luaL_checktype(L, -1, LUA_TTABLE);

    lua_getfield(L, -1, "Name");
    filter.Name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "Extensions");
    luaL_checktype(L, -1, LUA_TTABLE);
    int extensions = (int)lua_objlen(L, -1);
    if (extensions == 0) {
        luaL_error(L, "File dialog filter must include at least one extension");
    }
    for (int i = 1; i <= extensions; ++i) {
        lua_rawgeti(L, -1, i);
        const char* extension = luaL_checkstring(L, -1);
        if (extension[0] == '.') {
            ++extension;
        }
        filter.Extensions.emplace_back(extension);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    return filter;
}

FileDialogDescriptor getFileDialogDescriptor(lua_State* L, int index) {
    FileDialogDescriptor fileDialogDescriptor;
    luaL_checktype(L, index, LUA_TTABLE);

    lua_getfield(L, index, "Type");
    fileDialogDescriptor.Type = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    if (fileDialogDescriptor.Type != "FileDialog") {
        luaL_error(L, "File dialog descriptor must have a Type of FileDialog");
    }

    lua_getfield(L, index, "Mode");
    const char* mode = luaL_checkstring(L, -1);
    if (strcmp(mode, "OpenFile") == 0) {
        fileDialogDescriptor.Mode = FileDialogMode::OpenFile;
    } else if (strcmp(mode, "OpenFiles") == 0) {
        fileDialogDescriptor.Mode = FileDialogMode::OpenFiles;
    } else if (strcmp(mode, "OpenFolder") == 0) {
        fileDialogDescriptor.Mode = FileDialogMode::OpenFolder;
    } else if (strcmp(mode, "SaveFile") == 0) {
        fileDialogDescriptor.Mode = FileDialogMode::SaveFile;
    } else {
        luaL_error(L, "File dialog Mode must be OpenFile, OpenFiles, OpenFolder, or SaveFile");
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "Title");
    if (!lua_isnil(L, -1)) {
        fileDialogDescriptor.Title = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "InitialDirectory");
    if (!lua_isnil(L, -1)) {
        fileDialogDescriptor.InitialDirectory = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "DefaultFileName");
    if (!lua_isnil(L, -1)) {
        fileDialogDescriptor.DefaultFileName = luaL_checkstring(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, index, "Filters");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        int filters = (int)lua_objlen(L, -1);
        for (int i = 1; i <= filters; ++i) {
            lua_rawgeti(L, -1, i);
            fileDialogDescriptor.Filters.push_back(getFileDialogFilter(L));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    return fileDialogDescriptor;
}

} // namespace Luwow::Gui