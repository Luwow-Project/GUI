#include "TextArea.h"
#include "Handle.h"
#include "Window.h"
#include "ILuauHost.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(TextArea& textArea) {
    const TextAreaDescriptor& p = textArea.properties;
    textArea.control->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<TextArea>& TextArea::GetProperties() {
    static const Properties<TextArea> properties = {
        stringProperty<TextArea, &TextAreaDescriptor::Text>("Text", [](TextArea& textArea) {
            textArea.control->setText(textArea.properties.Text);
        }, false),
        pairProperty<TextArea, &TextAreaDescriptor::Width, &TextAreaDescriptor::Height>("Size", applyBounds),
        pairProperty<TextArea, &TextAreaDescriptor::Left, &TextAreaDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

const std::unordered_set<std::string>& TextArea::memberNames() const {
    return Handle<TextArea>::Members();
}

void TextArea::pushHandle(lua_State* L) {
    Handle<TextArea>::Push(L, shared_from_this());
}

void TextArea::readProperties(lua_State* L, int index) {
    readDescriptor<TextArea>(L, index, properties);
}

void TextArea::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void TextArea::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    (void)position;
    control.reset(module->createTextArea(properties, Window::containing(*parent)));

    std::weak_ptr<Node> weak = shared_from_this();
    control->onTextChanged = [weak] {
        std::shared_ptr<Node> node = weak.lock();
        Luwow::Engine::ILuauHost* host = nodeHost();
        if (!node || !node->alive || !host) return;
        Luwow::Engine::StateLock lock(host);
        static_cast<TextArea&>(*node).sync(host->getMainState());
    };
}

void TextArea::sync(lua_State* L) {
    TextAreaDescriptor before = properties;
    properties.Text = control->getText();
    fireChanges(L, *this, before);
}

} // namespace Luwow::Gui::ControlNode
