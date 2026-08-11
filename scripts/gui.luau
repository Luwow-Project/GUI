-- This is the type system for the GUI API as it exists in C++ code.
-- declare require: typeof(require) & ("@Luwow/gui") -> Gui

export type CommonWindowDescriptor = {
    Left: number?,
    Top: number?,
    Width: number?,
    Height: number?,
}

export type WindowDescriptor = CommonWindowDescriptor & {
    Type: "Window",
    Title: string,
}

export type ButtonDescriptor = CommonWindowDescriptor & {
    Type: "Button",
    Caption: string,
    OnPressed: () -> (),
}

export type MenuItemDescriptor = {
    Type: "MenuItem",
    Title: string,
    OnSelected: () -> (),
}

export type MenuDescriptor = {
    Type: "Menu",
    Title: string,
    Items: { MenuItemDescriptor },
}

export type MenuBarDescriptor = {
    Type: "MenuBar",
    Menus: { MenuDescriptor },
}

export type WindowDescriptors = WindowDescriptor | ButtonDescriptor | MenuBarDescriptor

export type Window = {
    show: () -> boolean,
    close: () -> (),
}

export type Button = {
}

export type MenuBar = {
}

export type Gui = {
    createWindow: (descriptor: WindowDescriptor) -> Window,
    createButton: (descriptor: ButtonDescriptor, parent: Window) -> Button,
    createMenuBar: (descriptor: MenuBarDescriptor, parent: Window) -> MenuBar,
}
return {} :: Gui
