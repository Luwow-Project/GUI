#include "TextBox.h"
#include "Handle.h"
#include "Window.h"
#include "ILuauHost.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(TextBox& textBox) {
    const TextBoxDescriptor& p = textBox.properties;
    textBox.native->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<TextBox>& TextBox::GetProperties() {
    static const Properties<TextBox> properties = {
        stringProperty<TextBox, &TextBoxDescriptor::Text>("Text", [](TextBox& textBox) {
            textBox.native->setText(textBox.properties.Text);
        }, false),
        pairProperty<TextBox, &TextBoxDescriptor::Width, &TextBoxDescriptor::Height>("Size", applyBounds),
        pairProperty<TextBox, &TextBoxDescriptor::Left, &TextBoxDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

void TextBox::pushHandle(lua_State* L) {
    Handle<TextBox>::Push(L, shared_from_this());
}

void TextBox::readProperties(lua_State* L, int index) {
    readDescriptor<TextBox>(L, index, properties);
}

void TextBox::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    (void)position;
    native.reset(module->createTextBox(properties, Window::containing(*parent)));

    std::weak_ptr<Control> weak = shared_from_this();
    native->onTextChanged = [weak] {
        std::shared_ptr<Control> control = weak.lock();
        Luwow::Engine::ILuauHost* host = controlHost();
        if (!control || !control->alive || !host) return;
        Luwow::Engine::StateLock lock(host);
        static_cast<TextBox&>(*control).sync(host->getMainState());
    };
}

void TextBox::sync(lua_State* L) {
    TextBoxDescriptor before = properties;
    properties.Text = native->getText();
    fireChanges(L, *this, before);
}

} // namespace Luwow::Gui::ControlNode
