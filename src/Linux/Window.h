#pragma once

#include "IGuiModule.h"

#include <gtkmm/box.h>
#include <gtkmm/fixed.h>
#include <gtkmm/window.h>
#include <sigc++/connection.h>
#include <memory>
#include <unordered_map>

namespace Luwow::Gui {

class Window : public IWindow {
public:
    Window(const WindowDescriptor& descriptor);
    ~Window() override;
    void show() override;
    void close() override;
    void* getNativeHandle() const override;
    uint16_t registerCommandControl(ICommandControl* commandControl) override;
    ICommandControl* getCommandControl(uint16_t id) const override;

    WindowDescriptor getDescriptor() const { return descriptor; }
    Gtk::Window* getGtkWindow() const { return gtkWindow.get(); }
    Gtk::Fixed* getFixed() const { return fixed; }
    void setMenuBar(Gtk::Widget& menuBar);

private:
    bool onDeleteEvent(GdkEventAny* event);
    void onHide();

    uint16_t nextCommandId = 1;
    std::unordered_map<uint16_t, ICommandControl*> commandControls;
    WindowDescriptor descriptor;
    std::unique_ptr<Gtk::Window> gtkWindow;
    Gtk::Box* vbox = nullptr;
    Gtk::Fixed* fixed = nullptr;
    sigc::connection deleteConnection;
    sigc::connection hideConnection;
};

} // namespace Luwow::Gui
