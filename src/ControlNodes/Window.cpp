#include "Window.h"
#include "Button.h"
#include "TextLabel.h"
#include "TextBox.h"
#include "TextArea.h"
#include "MenuBar.h"
#include "Handle.h"
#include "ILuauHost.h"

namespace Luwow::Gui::ControlNode {

namespace {

// The windows made, kept for the program's life. Never destroyed: at exit the platform may already be gone.
std::vector<std::shared_ptr<Node>>& windows() {
    static auto* list = new std::vector<std::shared_ptr<Node>>();
    return *list;
}

} // namespace

const Properties<Window>& Window::GetProperties() {
    static const Properties<Window> properties = {
        stringProperty<Window, &WindowDescriptor::Title>("Title", [](Window& window) {
            window.control->setTitle(window.properties.Title);
        }),
        pairProperty<Window, &WindowDescriptor::Left, &WindowDescriptor::Top>("Position", &Window::applyBounds),
        pairProperty<Window, &WindowDescriptor::Width, &WindowDescriptor::Height>("Size", &Window::applyBounds),
    };
    return properties;
}

void Window::pushHandle(lua_State* L) {
    Handle<Window>::Push(L, shared_from_this());
}

bool Window::acceptsChild(const Node& child) const {
    return dynamic_cast<const Button*>(&child)
        || dynamic_cast<const TextLabel*>(&child)
        || dynamic_cast<const TextBox*>(&child)
        || dynamic_cast<const TextArea*>(&child)
        || dynamic_cast<const MenuBar*>(&child);
}

void Window::readProperties(lua_State* L, int index) {
    readDescriptor<Window>(L, index, properties);
}

// A window has one menu bar at most
std::string Window::refuseChild(const Node& child) const {
    if (dynamic_cast<const MenuBar*>(&child)) {
        for (const std::shared_ptr<Node>& existing : children) {
            if (dynamic_cast<const MenuBar*>(existing.get())) return "a Window has only one MenuBar";
        }
    }
    return Node::refuseChild(child);
}

namespace {

const Window* rootWindow(const Node& node) {
    const Node* root = &node;
    while (root->parent) root = root->parent;
    return dynamic_cast<const Window*>(root);
}

} // namespace

IWindow* Window::containing(const Node& node) {
    const Window* window = rootWindow(node);
    if (!window || !window->control || !window->control->isOpen()) return nullptr;
    return window->control.get();
}

bool Window::isClosed(const Node& node) {
    const Window* window = rootWindow(node);
    return window && window->control && !window->control->isOpen();
}

int Window::show(lua_State* L, Window& window) {
    (void)L;
    window.control->show();
    return 0;
}

int Window::close(lua_State* L, Window& window) {
    (void)L;
    window.control->close();
    return 0;
}

void Window::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    (void)parent;
    (void)position;
    control.reset(module->createWindow(properties));
    windows().push_back(shared_from_this());
    readBounds();

    // Moved or resized by the user, or by the platform after setBounds
    std::weak_ptr<Node> weak = shared_from_this();
    control->onBoundsChanged = [weak] {
        std::shared_ptr<Node> node = weak.lock();
        Luwow::Engine::ILuauHost* host = nodeHost();
        if (!node || !node->alive || !host) return;
        Window& window = static_cast<Window&>(*node);
        if (window.applyingBounds) return;
        Luwow::Engine::StateLock lock(host);
        window.sync(host->getMainState());
    };
}

// The setters start from the bounds the window has now, so the other of Size and Position stays where
// the user left it
void Window::sync(lua_State* L) {
    WindowDescriptor before = properties;
    readBounds();
    fireChanges(L, *this, before);
}

void Window::applyBounds(Window& window) {
    const WindowDescriptor& p = window.properties;
    window.applyingBounds = true;
    window.control->setBounds(p.Left, p.Top, p.Width, p.Height);
    window.applyingBounds = false;
    window.readBounds();
}

void Window::readBounds() {
    if (!control || !control->isOpen()) return;
    control->getBounds(properties.Left, properties.Top, properties.Width, properties.Height);
}

} // namespace Luwow::Gui::ControlNode
