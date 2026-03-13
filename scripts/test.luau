local gui = require("gui.luau")

local window = gui.createWindow({
  Type = "Window" :: "Window",
  Title = "Luwow Created Window",
  Width = 400,
  Height = 300,
})

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

window.show()
