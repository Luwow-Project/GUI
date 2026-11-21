#pragma once

#include "IGuiModule.h"

#include <gtkmm/checkbutton.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Window;

class Checkbox : public ICheckbox {
public:
    Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent);
    ~Checkbox() override;
    void* getNativeHandle() const override { return check->gobj(); }

    void setCaption(const std::string& caption) override;
    void setChecked(bool checked) override;
    bool isChecked() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::CheckButton* check = nullptr;
    sigc::connection toggled;
    bool updating = false;
};

} // namespace Luwow::Gui
