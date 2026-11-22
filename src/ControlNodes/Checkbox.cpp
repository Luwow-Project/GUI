#include "Checkbox.h"
#include "Handle.h"
#include "Window.h"

namespace Luwow::Gui::ControlNode {

namespace {

void applyBounds(Checkbox& checkbox) {
    const CheckboxDescriptor& p = checkbox.properties;
    checkbox.native->setBounds(p.Left, p.Top, p.Width, p.Height);
}

} // namespace

const Properties<Checkbox>& Checkbox::GetProperties() {
    static const Properties<Checkbox> properties = {
        stringProperty<Checkbox, &CheckboxDescriptor::Caption>("Caption", [](Checkbox& checkbox) {
            checkbox.native->setCaption(checkbox.properties.Caption);
        }),
        boolProperty<Checkbox, &CheckboxDescriptor::Checked>("Checked", [](Checkbox& checkbox) {
            checkbox.native->setChecked(checkbox.properties.Checked);
        }),
        pairProperty<Checkbox, &CheckboxDescriptor::Width, &CheckboxDescriptor::Height>("Size", applyBounds),
        pairProperty<Checkbox, &CheckboxDescriptor::Left, &CheckboxDescriptor::Top>("Position", applyBounds),
    };
    return properties;
}

void Checkbox::pushHandle(lua_State* L) {
    Handle<Checkbox>::Push(L, shared_from_this());
}

void Checkbox::readProperties(lua_State* L, int index) {
    readDescriptor<Checkbox>(L, index, properties);
}

void Checkbox::createNative(lua_State* L, Control* parent, size_t position) {
    (void)L;
    (void)position;
    native.reset(module->createCheckbox(properties, Window::containing(*parent)));
    native->onCommand = syncHandler(*this);
}

void Checkbox::sync(lua_State* L) {
    CheckboxDescriptor before = properties;
    properties.Checked = native->isChecked();
    fireChanges(L, *this, before);
}

} // namespace Luwow::Gui::ControlNode
