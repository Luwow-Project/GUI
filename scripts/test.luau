local gui = require("@Luwow/gui")

local window = gui.createWindow({
  Type = "Window" :: "Window",
  Title = "Luwow Created Window",
  Width = 400,
  Height = 300,
})

local _menuBar = gui.createMenuBar({
  Type = "MenuBar" :: "MenuBar",
  Menus = {
    {
      Type = "Menu" :: "Menu",
      Title = "&File",
      Items = {
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "&Demo",
          OnSelected = function()
            print("Hello from the menu")
          end,
        },
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "&Open File...",
          OnSelected = function()
            local paths = gui.showFileDialog({
              Type = "FileDialog" :: "FileDialog",
              Mode = "OpenFile" :: "OpenFile",
              Title = "Open File",
              Filters = {
                { Name = "Text Files", Extensions = { "txt", "luau", "lua" } },
                { Name = "All Files", Extensions = { "*" } },
              },
            }, window)
            if paths then
              print("Selected file: " .. paths[1])
            else
              print("Open file cancelled")
            end
          end,
        },
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "&Open Files...",
          OnSelected = function()
            local paths = gui.showFileDialog({
              Type = "FileDialog" :: "FileDialog",
              Mode = "OpenFiles" :: "OpenFiles",
              Title = "Open Files",
              Filters = {
                { Name = "Text Files", Extensions = { "txt", "luau", "lua" } },
                { Name = "All Files", Extensions = { "*" } },
              },
            }, window)
            if paths then
                for i, v in paths do
                    print("Selected file: " .. v)
                end
            else
              print("Open files cancelled")
            end
          end,
        },
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "Open &Folder...",
          OnSelected = function()
            local paths = gui.showFileDialog({
              Type = "FileDialog" :: "FileDialog",
              Mode = "OpenFolder" :: "OpenFolder",
              Title = "Open Folder",
            }, window)
            if paths then
              print("Selected folder: " .. paths[1])
            else
              print("Open folder cancelled")
            end
          end,
        },
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "&Save As...",
          OnSelected = function()
            local paths = gui.showFileDialog({
              Type = "FileDialog" :: "FileDialog",
              Mode = "SaveFile" :: "SaveFile",
              Title = "Save As",
              DefaultFileName = "untitled.txt",
              Filters = {
                { Name = "Text Files", Extensions = { "txt" } },
                { Name = "All Files", Extensions = { "*" } },
              },
            }, window)
            if paths then
              print("Save path: " .. paths[1])
            else
              print("Save cancelled")
            end
          end,
        },
        {
          Type = "MenuItem" :: "MenuItem",
          Title = "&Quit",
          OnSelected = function()
            print("Quit selected")
            window:close()
          end,
        },
      },
    },
  },
}, window)

local numPresses = 0
local _button = gui.createButton({
  Type = "Button" :: "Button",
  Caption = "Click me",
  OnPressed = function()
    numPresses += 1
    print("Button pressed " .. numPresses .. " times")
  end,
  Width = 100,
  Height = 30,
  Left = 40,
  Top = 40,
}, window)

window:show()