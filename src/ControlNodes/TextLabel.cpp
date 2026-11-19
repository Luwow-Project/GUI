#include "TextLabel.h"
#include "Handle.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(TextLabel& label) {
    const TextLabelDescriptor& p = label.properties;
    label.control->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<TextLabel>& TextLabel::GetProperties() {
    static const Properties<TextLabel> properties = {
        stringProperty<TextLabel, &TextLabelDescriptor::Text>("Text", [](TextLabel& label) {
            label.control->setText(label.properties.Text);
        }),
        pairProperty<TextLabel, &TextLabelDescriptor::Width, &TextLabelDescriptor::Height>("Size", applyBounds),
        pairProperty<TextLabel, &TextLabelDescriptor::Left, &TextLabelDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

const std::unordered_set<std::string>& TextLabel::memberNames() const {
    return Handle<TextLabel>::Members();
}

void TextLabel::pushHandle(lua_State* L) {
    Handle<TextLabel>::Push(L, shared_from_this());
}

void TextLabel::readProperties(lua_State* L, int index) {
    readDescriptor<TextLabel>(L, index, properties);
}

void TextLabel::describe(lua_State* L) const {
    describeProperties(L, *this);
}

void TextLabel::createControl(lua_State* L, Node* parent, size_t position) {
    (void)L;
    (void)position;
    control.reset(module->createTextLabel(properties, Window::containing(*parent)));
}

} // namespace Luwow::Gui::ControlNode
