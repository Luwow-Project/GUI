#pragma once

#include "IGuiModule.h"

#include <gtkmm/comboboxtext.h>
#include <sigc++/connection.h>

namespace Luwow::Gui {

class Window;

class Dropdown : public IDropdown {
public:
    Dropdown(const DropdownDescriptor& descriptor, IWindow* parent);
    ~Dropdown() override;
    void* getNativeHandle() const override { return combo->gobj(); }

    void setItems(const std::vector<std::string>& items) override;
    void setSelected(int index) override;
    int getSelected() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    Gtk::ComboBoxText* combo = nullptr;
    sigc::connection changed;
    bool updating = false;
};

} // namespace Luwow::Gui
