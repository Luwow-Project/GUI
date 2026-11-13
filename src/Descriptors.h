#pragma once

#include <string>
#include <vector>

struct lua_State;

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
    std::string Title = "";
    int Width = 0;
    int Height = 0;
    int Left = 0;
    int Top = 0;
};

struct ButtonDescriptor {
    std::string Caption = "";
    int Width = 0;
    int Height = 0;
    int Left = 0;
    int Top = 0;
};

struct MenuBarDescriptor {};

struct MenuDescriptor {
    std::string Title = "";
};

struct MenuItemDescriptor {
    std::string Title = "";
};

// The two numbers of the {x, y} or {width, height} table at index, false if it isn't one
bool readPair(lua_State* L, int index, int& first, int& second);
// Pushes a new {first, second} table
void pushPair(lua_State* L, int first, int second);

// The file dialog descriptor, the table at index
FileDialogDescriptor getFileDialogDescriptor(lua_State* L, int index);

} // namespace Luwow::Gui
