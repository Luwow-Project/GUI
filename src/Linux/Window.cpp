#include "Window.h"
#include "GtkLoop.h"

#include <gdk/gdk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <stdexcept>

namespace Luwow::Gui {
namespace {

// Whether windows can be placed and their places read. Wayland gives neither.
bool positionsKnown() {
#ifdef GDK_WINDOWING_X11
    return GDK_IS_X11_DISPLAY(gdk_display_get_default());
#else
    return false;
#endif
}

} // namespace

Window::Window(const WindowDescriptor& descriptor)
    : left(descriptor.Left), top(descriptor.Top), width(descriptor.Width), height(descriptor.Height) {
    ensureGtkInitialized();

    gtkWindow = std::make_unique<Gtk::Window>();
    gtkWindow->set_title(descriptor.Title);
    if (width > 0 && height > 0) {
        gtkWindow->set_default_size(width, height);
    } else {
        gtkWindow->get_size(width, height);
    }
    gtkWindow->move(left, top);

    // The menu bar sits above the fixed area, so button coordinates are client
    // coordinates below the menu, matching the Win32 client area.
    vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    fixed = Gtk::manage(new Gtk::Fixed());
    vbox->pack_start(*fixed, true, true, 0);
    fixed->show();
    vbox->show();
    gtkWindow->add(*vbox);

    deleteConnection = gtkWindow->signal_delete_event().connect(sigc::mem_fun(*this, &Window::onDeleteEvent));
    configureConnection = gtkWindow->signal_configure_event().connect(sigc::mem_fun(*this, &Window::onConfigureEvent), false);
}

Window::~Window() {
    deleteConnection.disconnect();
    configureConnection.disconnect();
    markClosed();
}

void Window::markClosed() {
    if (!open) {
        return;
    }
    open = false;
    if (shown) {
        shown = false;
        windowClosed();
    }
}

bool Window::onDeleteEvent(GdkEventAny* event) {
    (void)event;
    close();
    return true;
}

// Moved or resized, by the user, the window manager or setBounds
bool Window::onConfigureEvent(GdkEventConfigure* event) {
    (void)event;
    if (!open) {
        return false;
    }
    if (positionsKnown()) {
        gtkWindow->get_position(left, top);
    }
    gtkWindow->get_size(width, height);
    if (onBoundsChanged) onBoundsChanged();
    return false;
}

uint16_t Window::registerCommandControl(ICommandControl* commandControl) {
    uint16_t id = nextCommandId++;
    commandControls[id] = commandControl;
    return id;
}

void Window::unregisterCommandControl(uint16_t id) {
    commandControls.erase(id);
}

ICommandControl* Window::getCommandControl(uint16_t id) const {
    auto it = commandControls.find(id);
    return it == commandControls.end() ? nullptr : it->second;
}

void Window::show() {
    if (!open) {
        return;
    }
    if (!shown) {
        shown = true;
        windowShown();
    }
    gtkWindow->show();
    gtkWindow->present();
    // Window managers place a new window themselves, so it's put back where it was asked to be
    gtkWindow->move(left, top);
}

// The window is hidden and stays until its node goes, its widgets with it
void Window::close() {
    if (!open) {
        return;
    }
    gtkWindow->hide();
    markClosed();
}

void* Window::getNativeHandle() const {
    return gtkWindow ? gtkWindow->gobj() : nullptr;
}

void Window::setTitle(const std::string& title) {
    if (open) gtkWindow->set_title(title);
}

// The position is the frame's top left, and the size the content's
void Window::setBounds(int left, int top, int width, int height) {
    if (!open) {
        return;
    }
    this->left = left;
    this->top = top;
    gtkWindow->move(left, top);
    if (width > 0 && height > 0) {
        this->width = width;
        this->height = height;
        gtkWindow->resize(width, height);
    }
}

void Window::getBounds(int& left, int& top, int& width, int& height) const {
    if (!open) {
        return;
    }
    left = this->left;
    top = this->top;
    width = this->width;
    height = this->height;
}

void Window::setMenuBar(Gtk::Widget& menuBar) {
    vbox->pack_start(menuBar, false, false, 0);
    vbox->reorder_child(menuBar, 0);
    menuBar.show();
}

} // namespace Luwow::Gui
