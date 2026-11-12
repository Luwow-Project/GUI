-- The type definitions of the GUI library as it exists in C++ code. Load this file as a definitions file in the LSP.

--[=[
	A handler's link to its signal.
]=]
export type GuiConnection = {
	read Connected: boolean,
	Disconnect: (self: GuiConnection) -> (),
}

--[=[
	An event that handlers connect to.
]=]
export type GuiSignal<T...> = {
	Connect: (self: GuiSignal<T...>, handler: (T...) -> ()) -> GuiConnection,
	Once: (self: GuiSignal<T...>, handler: (T...) -> ()) -> GuiConnection,
	Wait: (self: GuiSignal<T...>) -> T...
}

--[=[
	A Size's width and height, or a Position's x and y, in pixels. `{ 400, 300 }`
]=]
export type GuiVector = { number }

--[=[
	The descriptor types for each component.

	A name makes the component's node a field of its parent's, so it can't be a member of the parent,
	such as Title or add, and is unique among its siblings.
]=]

export type GuiWindowDescriptor = {
	Type: "Window",
	Name: string?,
	Title: string,
	Size: GuiVector?,
	Position: GuiVector?,
	Children: { [number | string]: GuiButtonDescriptor | GuiMenuBarDescriptor }?,
}

export type GuiButtonDescriptor = {
	Type: "Button",
	Name: string?,
	Order: number?,
	Caption: string,
	Size: GuiVector?,
	Position: GuiVector?,
	OnPressed: (() -> ())?
}

export type GuiMenuItemDescriptor = {
	Type: "MenuItem",
	Name: string?,
	Order: number?,
	Title: string,
	OnSelected: (() -> ())?
}

export type GuiMenuDescriptor = {
	Type: "Menu",
	Name: string?,
	Order: number?,
	Title: string,
	Children: { [number | string]: GuiMenuItemDescriptor }?,
	Items: { GuiMenuItemDescriptor }?
}

export type GuiMenuBarDescriptor = {
	Type: "MenuBar",
	Name: string?,
	Order: number?,
	Children: { [number | string]: GuiMenuDescriptor }?,
	Menus: { GuiMenuDescriptor }?
}

export type GuiDescriptor = GuiWindowDescriptor | GuiButtonDescriptor | GuiMenuBarDescriptor | GuiMenuDescriptor | GuiMenuItemDescriptor

export type GuiFileDialogFilter = {
	Name: string,
	Extensions: { string },
}

export type GuiFileDialogDescriptor = {
	Type: "FileDialog",
	Mode: "OpenFile" | "OpenFiles" | "OpenFolder" | "SaveFile",
	Title: string?,
	InitialDirectory: string?,
	DefaultFileName: string?,
	Filters: { GuiFileDialogFilter }?,
}

--[=[
	A component at runtime, made from its descriptor.
]=]
declare extern type GuiNode with
	read Name: string?
	-- The descriptor's Type
	read ClassName: string
	-- nil for a window
	read Parent: GuiNode?

	--[=[
		A descriptor of the node as it is now, its children's included as an array with their Names.
		Event handlers aren't in it.
	]=]
	function getDescription(self): GuiDescriptor

	--[=[
		The node's children, in order.
	]=]
	function getChildren(self): { GuiNode }

	--[=[
		A signal fired with the property's new value when it's set.

		@param property The name of one of the node's properties.
	]=]
	function bindToPropertyChanged(self, property: string): GuiSignal<any>
end

declare extern type GuiWindow extends GuiNode with
	Title: string
	Size: GuiVector
	Position: GuiVector

	--[=[
		Makes a Button or MenuBar descriptor into a new child node, its own Children too.

		@param descriptor The child's descriptor.
		@param at The child's 1-based position, or a child to put it before. The end when it's left out.
		@return The new child node.
	]=]
	function add(self, descriptor: GuiButtonDescriptor | GuiMenuBarDescriptor, at: (number | GuiNode)?): GuiButton | GuiMenuBar

	--[=[
		Takes a child and its children away. Their nodes can't be used after.

		@param child The child node, its Name, or its 1-based position.
	]=]
	function remove(self, child: GuiNode | string | number): ()

	--[=[
		Shows the window.
	]=]
	function show(self): ()

	--[=[
		Closes the window. Its nodes stay, but nothing can be added to it.
	]=]
	function close(self): ()
end

declare extern type GuiButton extends GuiNode with
	Caption: string
	Size: GuiVector
	Position: GuiVector
	-- Fired when the button is pressed
	read OnPressed: GuiSignal<>
end

declare extern type GuiMenuBar extends GuiNode with
	--[=[
		Makes a Menu descriptor into a new child node, its own Children too.

		@param descriptor The menu's descriptor.
		@param at The menu's 1-based position, or a menu to put it before. The end when it's left out.
		@return The new Menu node.
	]=]
	function add(self, descriptor: GuiMenuDescriptor, at: (number | GuiNode)?): GuiMenu

	--[=[
		Takes a menu and its items away. Their nodes can't be used after.

		@param child The menu node, its Name, or its 1-based position.
	]=]
	function remove(self, child: GuiNode | string | number): ()
end

declare extern type GuiMenu extends GuiNode with
	Title: string

	--[=[
		Makes a MenuItem descriptor into a new child node.

		@param descriptor The item's descriptor.
		@param at The item's 1-based position, or an item to put it before. The end when it's left out.
		@return The new MenuItem node.
	]=]
	function add(self, descriptor: GuiMenuItemDescriptor, at: (number | GuiNode)?): GuiMenuItem

	--[=[
		Takes an item away. Its node can't be used after.

		@param child The item node, its Name, or its 1-based position.
	]=]
	function remove(self, child: GuiNode | string | number): ()
end

declare extern type GuiMenuItem extends GuiNode with
	Title: string
	-- Fired when the item is selected
	read OnSelected: GuiSignal<>
end

--[=[
	Every component the library makes.
]=]
export type GuiComponents = {
	Window: { Node: GuiWindow, Descriptor: GuiWindowDescriptor },
	Button: { Node: GuiButton, Descriptor: GuiButtonDescriptor },
	MenuBar: { Node: GuiMenuBar, Descriptor: GuiMenuBarDescriptor },
	Menu: { Node: GuiMenu, Descriptor: GuiMenuDescriptor },
	MenuItem: { Node: GuiMenuItem, Descriptor: GuiMenuItemDescriptor },
}

--[=[
	The node a descriptor of the given kind makes, with its keyed children as typed fields down the whole tree.
]=]
export type function GuiNodeTree(descriptor, components, kind)
	local function withoutNil(candidate: type): { type }
		local parts = {}
		if candidate:is("union") then
			for _, component in candidate:components() do
				if not component:is("nil") then
					table.insert(parts, component)
				end
			end
		else
			table.insert(parts, candidate)
		end
		return parts
	end

	local function tableIn(candidate: type?): type?
		if not candidate then
			return nil
		end
		if candidate:is("table") then
			return candidate
		end
		if candidate:is("union") or candidate:is("intersection") then
			for _, component in candidate:components() do
				if component:is("table") then
					return component
				end
			end
		end
		return nil
	end

	local function field(source: type, name: string): type?
		return source:readproperty(types.singleton(name))
	end

	local function stringValue(candidate: type?): string?
		if candidate and candidate:is("singleton") then
			local value = candidate:value()
			if type(value) == "string" then
				return value
			end
		end
		return nil
	end

	-- The sorts of value a type takes: "string", "number", "table" and so on, or "any"
	local function sortsOf(candidate: type): { [string]: boolean }
		local sorts = {}
		for _, part in withoutNil(candidate) do
			if part:is("any") or part:is("unknown") or part:is("union") or part:is("intersection") then
				sorts.any = true
			elseif part:is("singleton") then
				sorts[type(part:value())] = true
			else
				sorts[part.tag] = true
			end
		end
		return sorts
	end

	local function isOptional(candidate: type): boolean
		if candidate:is("nil") then
			return true
		end
		if candidate:is("union") then
			for _, component in candidate:components() do
				if component:is("nil") then
					return true
				end
			end
		end
		return false
	end

	local function describe(sorts: { [string]: boolean }): string
		local names = {}
		for sort in sorts do
			table.insert(names, sort)
		end
		table.sort(names)
		return table.concat(names, " or ")
	end

	local function component(name: string): (type, type)
		local entry = tableIn(field(components, name))
		local node = entry and field(entry, "Node")
		local declared = entry and tableIn(field(entry, "Descriptor"))
		if not node or not declared then
			error(`{name} isn't one of the components`)
		end
		return node, declared
	end

	-- The kinds a descriptor type's Children holds, by their descriptors' Types
	local function childKinds(declared: type): { string }
		local kinds = {}
		local children = tableIn(field(declared, "Children"))
		local indexer = children and children:indexer()
		if not indexer then
			return kinds
		end
		for _, part in withoutNil(indexer.readresult) do
			local childDescriptor = tableIn(part)
			local childKind = childDescriptor and stringValue(field(childDescriptor, "Type"))
			if childKind then
				table.insert(kinds, childKind)
			end
		end
		table.sort(kinds)
		return kinds
	end

	-- Why the source doesn't fit the kind's descriptor, nil when it does. Its Children are checked as they're built.
	local function misfit(source: type, kindName: string, declared: type): string?
		for key, property in source:properties() do
			local name = key:value()
			if type(name) ~= "string" or name == "Type" then
				continue
			end
			local expected = field(declared, name)
			if not expected then
				return `{kindName} descriptors have no field '{name}'`
			end
			if property.read and name ~= "Children" then
				local given = sortsOf(property.read)
				local allowed = sortsOf(expected)
				if not given.any and not allowed.any then
					for sort in given do
						if not allowed[sort] then
							return `a {kindName}'s {name} must be a {describe(allowed)}`
						end
					end
				end
			end
		end
		for key, property in declared:properties() do
			local name = key:value()
			if name ~= "Type" and property.read and not isOptional(property.read) and not source:readproperty(key) then
				return `a {kindName} descriptor needs a {name}`
			end
		end
		return nil
	end

	local function build(source: type, kindName: string): type
		local node, declared = component(kindName)
		local problem = misfit(source, kindName, declared)
		if problem then
			error(problem)
		end

		local children = tableIn(field(source, "Children"))
		if not children then
			return node
		end

		local kinds = childKinds(declared)
		local named: { [type]: { read: type } } = {}
		local any = false
		for key, property in children:properties() do
			local child = tableIn(property.read)
			if not child then
				continue
			end

			local chosen = stringValue(field(child, "Type"))
			if chosen then
				if not table.find(kinds, chosen) then
					error(`a {chosen} can't be a child of a {kindName}`)
				end
			else
				local reasons = {}
				for _, candidate in kinds do
					local _, candidateDescriptor = component(candidate)
					local reason = misfit(child, candidate, candidateDescriptor)
					if not reason then
						chosen = candidate
						break
					end
					table.insert(reasons, reason)
				end
				if not chosen then
					error(`the {kindName}'s child '{tostring(key:value())}' fits none of its kinds: {table.concat(reasons, "; ")}`)
				end
			end

			named[key] = { read = build(child, chosen :: string) }
			any = true
		end
		if not any then
			return node
		end
		return types.intersectionof(node, types.newtable(named :: any))
	end

	local kindName = stringValue(kind)
	if not kindName then
		error("the kind must be a component's Type")
	end
	local source = tableIn(descriptor)
	if not source then
		return (component(kindName))
	end
	return build(source, kindName)
end

--[=[
	The Window node of a window descriptor, with the keyed children of its tree as typed fields.
]=]
export type GuiWindowOf<D> = GuiNodeTree<D, GuiComponents, "Window">

--[=[
	The gui library.
]=]
export type GuiLibrary = {
	--[=[
		Creates a new Window with the given descriptor, and its Children.

		@param descriptor The window descriptor that defines the properties of the window.
		@return The new Window object, its keyed children typed fields of it.
	]=]
	createWindow: <D>(descriptor: D) -> GuiWindowOf<D>,

	--[=[
		Shows a native file or folder picker dialog.

		@param descriptor The file dialog descriptor that defines the picker mode and options.
		@param parent An optional parent Window used as the dialog owner.
		@return A list of selected paths, or nil if the dialog was cancelled.
	]=]
	showFileDialog: (descriptor: GuiFileDialogDescriptor, parent: GuiWindow?) -> { string }?,
}
