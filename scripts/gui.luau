-- This is the type system for the GUI API as it exists in C++ code.

--[=[
	A handler's link to its signal.
]=]
export type Connection = {
	read Connected: boolean,
	Disconnect: (self: Connection) -> (),
}

--[=[
	An event that handlers connect to. T... are the values it's fired with.
]=]
export type Signal<T...> = {
	Connect: (self: Signal<T...>, handler: (T...) -> ()) -> Connection,
	Once: (self: Signal<T...>, handler: (T...) -> ()) -> Connection,
	Wait: (self: Signal<T...>) -> T...,
}

--[=[
	A Size's width and height, or a Position's x and y, in pixels. `{ 400, 300 }`
]=]
export type Vector = { number }

--[=[
	The descriptor types for each component, which gui.createComponent and add take. Their fields are read-only,
	so a table literal matches the overload of its Type.

	A name is unique among its siblings, and remove takes it in place of the child.
]=]

--[=[
	A descriptor's Children: an array of descriptors, descriptors keyed by their Names, or both.
]=]
export type Children<D> = { read [number]: D } | { read [string]: D }

export type WindowDescriptor = {
	read Type: "Window",
	read Name: string?,
	read Title: string,
	read Size: Vector?,
	read Position: Vector?,
	read Children: Children<ButtonDescriptor | TextLabelDescriptor | TextBoxDescriptor | TextAreaDescriptor | MenuBarDescriptor>?,
}

export type ButtonDescriptor = {
	read Type: "Button",
	read Name: string?,
	read Order: number?,
	read Caption: string,
	read Size: Vector?,
	read Position: Vector?,
	-- Connected to OnPressed
	read OnPressed: (() -> ())?,
}

export type TextLabelDescriptor = {
	read Type: "TextLabel",
	read Name: string?,
	read Order: number?,
	read Text: string,
	read Size: Vector?,
	read Position: Vector?,
}

export type TextBoxDescriptor = {
	read Type: "TextBox",
	read Name: string?,
	read Order: number?,
	read Text: string?,
	read Size: Vector?,
	read Position: Vector?,
}

export type TextAreaDescriptor = {
	read Type: "TextArea",
	read Name: string?,
	read Order: number?,
	read Text: string?,
	read Size: Vector?,
	read Position: Vector?,
}

export type MenuItemDescriptor = {
	read Type: "MenuItem",
	read Name: string?,
	read Order: number?,
	read Title: string,
	-- Connected to OnSelected
	read OnSelected: (() -> ())?,
}

export type MenuDescriptor = {
	read Type: "Menu",
	read Name: string?,
	read Order: number?,
	read Title: string,
	read Children: Children<MenuItemDescriptor>?,
}

export type MenuBarDescriptor = {
	read Type: "MenuBar",
	read Name: string?,
	read Order: number?,
	read Children: Children<MenuDescriptor>?,
}

export type Descriptor = WindowDescriptor | ButtonDescriptor | TextLabelDescriptor | TextBoxDescriptor | TextAreaDescriptor | MenuBarDescriptor | MenuDescriptor | MenuItemDescriptor

export type FileDialogFilter = {
	Name: string,
	Extensions: { string },
}

export type FileDialogDescriptor = {
	Type: "FileDialog",
	Mode: "OpenFile" | "OpenFiles" | "OpenFolder" | "SaveFile",
	Title: string?,
	InitialDirectory: string?,
	DefaultFileName: string?,
	Filters: { FileDialogFilter }?,
}

--[=[
	A component at runtime, made from its descriptor. Every other node type is a Node and its own members.
	A node has no Parent while it's detached, until it's added to another.
]=]
export type Node = {
	read Name: string?,
	-- The descriptor's Type
	read ClassName: string,
	-- nil for a window
	read Parent: Node?,

	--[=[
		The node's children, in order.
	]=]
	getChildren: (self: Node) -> { Node },

	--[=[
		The node's children of a class, in order.

		@param className The ClassName of the children, a descriptor Type such as "Button".
	]=]
	getChildrenWhichIsA: ((self: Node, className: "Window") -> { Window })
		& ((self: Node, className: "Button") -> { Button })
		& ((self: Node, className: "TextLabel") -> { TextLabel })
		& ((self: Node, className: "TextBox") -> { TextBox })
		& ((self: Node, className: "TextArea") -> { TextArea })
		& ((self: Node, className: "MenuBar") -> { MenuBar })
		& ((self: Node, className: "Menu") -> { Menu })
		& ((self: Node, className: "MenuItem") -> { MenuItem }),

	--[=[
		A signal fired with the property's new value when it's set.

		@param property The name of one of the node's properties.
	]=]
	bindToPropertyChanged: (self: Node, property: string) -> Signal<any>,
}

export type Window = Node & {
	Title: string,
	Size: Vector,
	Position: Vector,

	--[=[
		Adds a Button, text control or MenuBar: a descriptor made into a new node, its own Children too, or a
		detached node from gui.createComponent.

		@param child The child's descriptor, or a detached node.
		@param at The child's 1-based position, or a child to put it before. The end when it's left out.
		@return The child node. The type of a node made with gui.createComponent is exact, so a script that needs it
		can add that node.
	]=]
	add: (
		self: Window,
		child: ButtonDescriptor | Button | TextLabelDescriptor | TextLabel | TextBoxDescriptor | TextBox | TextAreaDescriptor | TextArea | MenuBarDescriptor | MenuBar,
		at: (number | Node)?
	) -> Button | TextLabel | TextBox | TextArea | MenuBar,

	--[=[
		Takes a child and its children away. Their nodes can't be used after.

		@param child The child node, its Name, or its 1-based position.
	]=]
	remove: (self: Window, child: Node | string | number) -> (),

	--[=[
		Shows the window.
	]=]
	show: (self: Window) -> (),

	--[=[
		Closes the window. Its nodes stay, but nothing can be added to it.
	]=]
	close: (self: Window) -> (),
}

export type Button = Node & {
	Caption: string,
	Size: Vector,
	Position: Vector,
	-- Fired when the button is pressed
	read OnPressed: Signal<>,
}

export type TextLabel = Node & {
	Text: string,
	Size: Vector,
	Position: Vector,
}

export type TextBox = Node & {
	Text: string,
	Size: Vector,
	Position: Vector,
}

export type TextArea = Node & {
	Text: string,
	Size: Vector,
	Position: Vector,
}

export type MenuBar = Node & {
	--[=[
		Adds a Menu: a descriptor made into a new node, its own Children too, or a detached Menu.

		@param child The menu's descriptor, or a detached Menu.
		@param at The menu's 1-based position, or a menu to put it before. The end when it's left out.
		@return The Menu node.
	]=]
	add: (
		self: MenuBar,
		child: MenuDescriptor | Menu,
		at: (number | Node)?
	) -> Menu,

	--[=[
		Takes a menu and its items away. Their nodes can't be used after.

		@param child The menu node, its Name, or its 1-based position.
	]=]
	remove: (self: MenuBar, child: Node | string | number) -> (),
}

export type Menu = Node & {
	Title: string,

	--[=[
		Adds a MenuItem: a descriptor made into a new node, or a detached MenuItem.

		@param child The item's descriptor, or a detached MenuItem.
		@param at The item's 1-based position, or an item to put it before. The end when it's left out.
		@return The MenuItem node.
	]=]
	add: (
		self: Menu,
		child: MenuItemDescriptor | MenuItem,
		at: (number | Node)?
	) -> MenuItem,

	--[=[
		Takes an item away. Its node can't be used after.

		@param child The item node, its Name, or its 1-based position.
	]=]
	remove: (self: Menu, child: Node | string | number) -> (),
}

export type MenuItem = Node & {
	Title: string,
	-- Fired when the item is selected
	read OnSelected: Signal<>,
}

local gui = {}

--[=[
	Creates the component of the descriptor's Type, with its Children. A Window is made at once. Any other
	component is detached, with no parent, until it's added to a node with add, and appears once it's in a
	window's tree. A detached node's properties can be set meanwhile.

	@param descriptor The descriptor of any component.
	@return The new node.
]=]
gui.createComponent = (nil :: any) :: ((descriptor: WindowDescriptor) -> Window)
	& ((descriptor: ButtonDescriptor) -> Button)
	& ((descriptor: TextLabelDescriptor) -> TextLabel)
	& ((descriptor: TextBoxDescriptor) -> TextBox)
	& ((descriptor: TextAreaDescriptor) -> TextArea)
	& ((descriptor: MenuBarDescriptor) -> MenuBar)
	& ((descriptor: MenuDescriptor) -> Menu)
	& ((descriptor: MenuItemDescriptor) -> MenuItem)
	& ((descriptor: Descriptor) -> Node)

--[=[
	Shows a native file or folder picker dialog.

	@param descriptor The file dialog descriptor that defines the picker mode and options.
	@param parent An optional parent Window used as the dialog owner.
	@return A list of selected paths, or nil if the dialog was cancelled.
]=]
gui.showFileDialog = (nil :: any) :: (descriptor: FileDialogDescriptor, parent: Window?) -> { string }?

export type Gui = typeof(gui)

return table.freeze(gui)
