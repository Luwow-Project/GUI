#include "Button.h"
#include "Handle.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(Button& button) {
    const ButtonDescriptor& p = button.properties;
    button.native->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<Button>& Button::GetProperties() {
    static const Properties<Button> properties = {
        stringProperty<Button, &ButtonDescriptor::Caption>("Caption", [](Button& button) {
            button.native->setCaption(button.properties.Caption);
        }),
        pairProperty<Button, &ButtonDescriptor::Width, &ButtonDescriptor::Height>("Size", applyBounds),
        pairProperty<Button, &ButtonDescriptor::Left, &ButtonDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

void Button::pushHandle(lua_State* L) {
    Handle<Button>::Push(L, shared_from_this());
}

void Button::readProperties(lua_State* L, int index) {
    readDescriptor<Button>(L, index, properties);
}

void Button::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    (void)position;
    native.reset(module->createButton(properties, Window::containing(*parent)));
}

} // namespace Luwow::Gui::ControlNode
