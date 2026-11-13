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
    bool isOpen() const override { return open; }
    void* getNativeHandle() const override;
    void setTitle(const std::string& title) override;
    void setBounds(int left, int top, int width, int height) override;
    void getBounds(int& left, int& top, int& width, int& height) const override;
    uint16_t registerCommandControl(ICommandControl* commandControl) override;
    void unregisterCommandControl(uint16_t id) override;
    ICommandControl* getCommandControl(uint16_t id) const override;

    Gtk::Window* getGtkWindow() const { return gtkWindow.get(); }
    Gtk::Fixed* getFixed() const { return fixed; }
    void setMenuBar(Gtk::Widget& menuBar);

private:
    bool onDeleteEvent(GdkEventAny* event);
    bool onConfigureEvent(GdkEventConfigure* event);
    // Marks the window closed, ending the loop with the last shown one
    void markClosed();

    uint16_t nextCommandId = 1;
    std::unordered_map<uint16_t, ICommandControl*> commandControls;
    std::unique_ptr<Gtk::Window> gtkWindow;
    Gtk::Box* vbox = nullptr;
    Gtk::Fixed* fixed = nullptr;
    sigc::connection deleteConnection;
    sigc::connection configureConnection;
    bool open = true;
    // Shown and not yet closed, one of the windows the loop runs for
    bool shown = false;
    // GTK moves and resizes when the window manager gets to it, so the bounds last set are kept until
    // the window reports the ones it has
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
};

// The main loop's count of shown windows. Closing the last one ends the loop, and a window that's never
// shown doesn't keep it running.
void windowShown();
void windowClosed();

} // namespace Luwow::Gui
