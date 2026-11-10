#include "Window.h"
#include "GtkLoop.h"

#include <stdexcept>

namespace Luwow::Gui {

Window::Window(const WindowDescriptor& descriptor) : descriptor(descriptor) {
    ensureGtkInitialized();

    gtkWindow = std::make_unique<Gtk::Window>();
    gtkWindow->set_title(descriptor.Title);
    gtkWindow->set_default_size(descriptor.Width, descriptor.Height);
    gtkWindow->move(descriptor.Left, descriptor.Top);

    // The menu bar sits above the fixed area, so button coordinates are client
    // coordinates below the menu, matching the Win32 client area.
    vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    fixed = Gtk::manage(new Gtk::Fixed());
    vbox->pack_start(*fixed, true, true, 0);
    fixed->show();
    vbox->show();
    gtkWindow->add(*vbox);

    deleteConnection = gtkWindow->signal_delete_event().connect(sigc::mem_fun(*this, &Window::onDeleteEvent));
    hideConnection = gtkWindow->signal_hide().connect(sigc::mem_fun(*this, &Window::onHide));
}

Window::~Window() {
    deleteConnection.disconnect();
    hideConnection.disconnect();
}

bool Window::onDeleteEvent(GdkEventAny* event) {
    (void)event;
    close();
    return true;
}

void Window::onHide() {
    requestGtkQuit();
}

uint16_t Window::registerCommandControl(ICommandControl* commandControl) {
    uint16_t id = nextCommandId++;
    commandControls[id] = commandControl;
    return id;
}

ICommandControl* Window::getCommandControl(uint16_t id) const {
    return commandControls.at(id);
}

void Window::show() {
    if (!gtkWindow) {
        throw std::runtime_error("Window not found");
    }
    gtkWindow->show_all();
    gtkWindow->present();
    gtkWindow->move(descriptor.Left, descriptor.Top);
}

void Window::close() {
    if (!gtkWindow) {
        throw std::runtime_error("Window not found");
    }
    // Hiding a visible window emits signal_hide, which ends the loop. Hiding
    // one that was never shown does not, so request the quit directly.
    const bool visible = gtkWindow->get_visible();
    gtkWindow->hide();
    if (!visible) {
        requestGtkQuit();
    }
}

void* Window::getNativeHandle() const {
    return gtkWindow ? gtkWindow->gobj() : nullptr;
}

void Window::setMenuBar(Gtk::Widget& menuBar) {
    if (!vbox) {
        throw std::runtime_error("Failed to assign menu bar to window");
    }
    vbox->pack_start(menuBar, false, false, 0);
    vbox->reorder_child(menuBar, 0);
    menuBar.show_all();
}

} // namespace Luwow::Gui
