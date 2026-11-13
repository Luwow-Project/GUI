#pragma once

#include "Node.h"
#include "Property.h"

namespace Luwow::Gui::ControlNode {

// A window, the root of a tree, with buttons and a menu bar as its children
class Window : public Node {
public:
    static constexpr const char* TypeName = "Window";
    static const Properties<Window>& GetProperties();

    template <typename H>
    static std::vector<typename H::MethodDef> Methods() {
        return {
            { "show", &H::template invoke<&Window::show> },
            { "close", &H::template invoke<&Window::close> },
        };
    }

    // gui.createWindow(descriptor), and the Window node of its descriptor's tree
    static int create(lua_State* L, IGuiModule* gui);
    // The control of the window the node is in, nullptr once it's closed
    static IWindow* containing(const Node& node);

    using Node::Node;
    const char* typeName() const override { return TypeName; }
    const std::unordered_set<std::string>& memberNames() const override;
    void pushHandle(lua_State* L) override;
    bool takesChildren() const override { return true; }
    bool acceptsChild(const Node& child) const override;
    void readProperties(lua_State* L, int index) override;
    void describe(lua_State* L) const override;
    void createControl(lua_State* L, Node* parent, size_t position) override;
    void destroyControl() override { control.reset(); }
    void sync(lua_State* L) override;

    WindowDescriptor properties;
    std::unique_ptr<IWindow> control;

private:
    static int show(lua_State* L, Window& window);
    static int close(lua_State* L, Window& window);
    // Gives the control the bounds set, then reads back the ones it took
    static void applyBounds(Window& window);
    // Reads the window's bounds as they are on screen into its properties
    void readBounds();

    // Set while the window's own setBounds runs, whose changes the setter fires itself
    bool applyingBounds = false;
};

} // namespace Luwow::Gui::ControlNode
