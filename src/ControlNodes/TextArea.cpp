#include "TextArea.h"
#include "Handle.h"
#include "Window.h"
#include "ILuauHost.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(TextArea& textArea) {
    const TextAreaDescriptor& p = textArea.properties;
    textArea.native->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<TextArea>& TextArea::GetProperties() {
    static const Properties<TextArea> properties = {
        stringProperty<TextArea, &TextAreaDescriptor::Text>("Text", [](TextArea& textArea) {
            textArea.native->setText(textArea.properties.Text);
        }, false),
        pairProperty<TextArea, &TextAreaDescriptor::Width, &TextAreaDescriptor::Height>("Size", applyBounds),
        pairProperty<TextArea, &TextAreaDescriptor::Left, &TextAreaDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

void TextArea::pushHandle(lua_State* L) {
    Handle<TextArea>::Push(L, shared_from_this());
}

void TextArea::readProperties(lua_State* L, int index) {
    readDescriptor<TextArea>(L, index, properties);
}

void TextArea::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    (void)position;
    native.reset(module->createTextArea(properties, Window::containing(*parent)));

    std::weak_ptr<Control> weak = shared_from_this();
    native->onTextChanged = [weak] {
        std::shared_ptr<Control> control = weak.lock();
        Luwow::Engine::ILuauHost* host = controlHost();
        if (!control || !control->alive || !host) return;
        Luwow::Engine::StateLock lock(host);
        static_cast<TextArea&>(*control).sync(host->getMainState());
    };
}

void TextArea::sync(lua_State* L) {
    TextAreaDescriptor before = properties;
    properties.Text = native->getText();
    fireChanges(L, *this, before);
}

} // namespace Luwow::Gui::ControlNode
