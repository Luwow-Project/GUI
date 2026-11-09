#pragma once

#include <string>
#include <vector>
#include <functional>
#include "lua.h"

namespace Luwow::Engine {
class ILuauHost;
}

namespace Luwow::Gui {

enum class FileDialogMode {
    OpenFile,
    OpenFiles,
    OpenFolder,
    SaveFile,
};

struct FileDialogFilter {
    std::string Name;
    std::vector<std::string> Extensions;
};

struct FileDialogDescriptor {
    std::string Type = "";
    FileDialogMode Mode = FileDialogMode::OpenFile;
    std::string Title = "";
    std::string InitialDirectory = "";
    std::string DefaultFileName = "";
    std::vector<FileDialogFilter> Filters;
};

struct WindowDescriptor {
    std::string Type = "";
    std::string Title = "";
    int Width = 0;
    int Height = 0;
    int Left = 0;
    int Top = 0;
};

struct ButtonDescriptor {
    std::string Type = "";
    std::string Caption = "";
    lua_State* L = nullptr;           // Lua state for callback
    Luwow::Engine::ILuauHost* host = nullptr; // Host whose state lock guards the callback
    int OnPressedRef = LUA_NOREF;     // Lua registry reference to callback function
    int Width = 0;
    int Height = 0;
    int Left = 0;
    int Top = 0;
};

struct TextBoxDescriptor {
    std::string Type = "";
    std::string Text = "";
    std::string Font = "";
    bool MultiLineMode = false;
    int Width = 0;
    int Height = 0;
    int Left = 0;
    int Top = 0;
};

struct MenuItemDescriptor {
    std::string Type = "";
    std::string Title = "";
    lua_State* L = nullptr;
    Luwow::Engine::ILuauHost* host = nullptr;
    int OnSelectedRef = LUA_NOREF;
};

struct MenuDescriptor {
    std::string Type = "";
    std::string Title = "";
    std::vector<MenuItemDescriptor> Items;
};

struct MenuBarDescriptor {
    std::string Type = "";
    std::vector<MenuDescriptor> Menus;
};

// Methods for getting the descriptors from their respective Luau tables.
WindowDescriptor getWindowDescriptor(lua_State* L);
ButtonDescriptor getButtonDescriptor(lua_State* L);
TextBoxDescriptor getTextBoxDescriptor(lua_State* L);
MenuItemDescriptor getMenuItemDescriptor(lua_State* L);
MenuDescriptor getMenuDescriptor(lua_State* L);
MenuBarDescriptor getMenuBarDescriptor(lua_State* L);
FileDialogDescriptor getFileDialogDescriptor(lua_State* L);

} // namespace Luwow::Gui