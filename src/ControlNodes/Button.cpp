#include "Button.h"
#include "Handle.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(Button& button) {
    const ButtonDescriptor& p = button.properties;
    button.control->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<Button>& Button::GetProperties() {
    static const Properties<Button> properties = {
        stringProperty<Button, &ButtonDescriptor::Caption>("Caption", [](Button& button) {
            button.control->setCaption(button.properties.Caption);
        }),
        pairProperty<Button, &ButtonDescriptor::Width, &ButtonDescriptor::Height>("Size", applyBounds),
        pairProperty<Button, &ButtonDescriptor::Left, &ButtonDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

const std::unordered_set<std::string>& Button::memberNames() const {
    return Handle<Button>::Members();
}

void Button::pushHandle(lua_State* L) {
    Handle<Button>::Push(L, shared_from_this());
}

void Button::readProperties(lua_State* L, int index) {
    readDescriptor<Button>(L, index, properties);
}

void Button::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void Button::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    (void)position;
    control.reset(module->createButton(properties, Window::containing(*parent)));
}

} // namespace Luwow::Gui::ControlNode
