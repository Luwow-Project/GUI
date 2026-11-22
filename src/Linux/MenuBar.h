#pragma once

#include "IGuiModule.h"

#include <gtkmm/menubar.h>

namespace Luwow::Gui {

class Window;

class MenuBar : public IMenuBar {
public:
    explicit MenuBar(IWindow* parent);
    ~MenuBar() override;
    void* getNativeHandle() const override { return menuBar->gobj(); }

    void insertMenu(size_t index, IMenu* menu) override;

private:
    Gtk::MenuBar* menuBar = nullptr;
};

} // namespace Luwow::Gui
