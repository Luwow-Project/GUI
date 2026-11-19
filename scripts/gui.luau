export type Connection = GuiConnection
export type Signal<T...> = GuiSignal<T...>
export type Vector = GuiVector

export type WindowDescriptor = GuiWindowDescriptor
export type ButtonDescriptor = GuiButtonDescriptor
export type TextLabelDescriptor = GuiTextLabelDescriptor
export type TextBoxDescriptor = GuiTextBoxDescriptor
export type TextAreaDescriptor = GuiTextAreaDescriptor
export type MenuBarDescriptor = GuiMenuBarDescriptor
export type MenuDescriptor = GuiMenuDescriptor
export type MenuItemDescriptor = GuiMenuItemDescriptor
export type Descriptor = GuiDescriptor
export type FileDialogFilter = GuiFileDialogFilter
export type FileDialogDescriptor = GuiFileDialogDescriptor

export type Node = GuiNode
export type Window = GuiWindow
export type Button = GuiButton
export type TextLabel = GuiTextLabel
export type TextBox = GuiTextBox
export type TextArea = GuiTextArea
export type MenuBar = GuiMenuBar
export type Menu = GuiMenu
export type MenuItem = GuiMenuItem
export type WindowOf<D> = GuiWindowOf<D>
export type Gui = GuiLibrary

return table.freeze({} :: GuiLibrary)
