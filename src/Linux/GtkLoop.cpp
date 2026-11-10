#include "GtkLoop.h"

#include <gtk/gtk.h>
#include <gtkmm/main.h>
#include <glibmm/main.h>
#include <stdexcept>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Luwow::Gui {
namespace {

// Leaked on purpose. Gtk::Main is the gtkmm 3 init object, and destroying it
// during static teardown shuts GTK down while script windows are still alive.
// Gtk::Application replaced it, but that API owns startup. This library enters
// the loop only after a script has already created its windows.
Gtk::Main*& gtkMain() {
    static Gtk::Main* main = nullptr;
    return main;
}

bool& quitRequested() {
    static bool requested = false;
    return requested;
}

} // namespace

void ensureGtkInitialized() {
    if (gtkMain()) {
        return;
    }

    static int argc = 1;
    static char appName[] = "luwow";
    static char* argvStorage[] = { appName, nullptr };
    char** argv = argvStorage;

    // gtk_init() exits the process when there is no display. Check first so
    // the host can report the failure.
    if (!gtk_init_check(&argc, &argv)) {
        throw std::runtime_error("Failed to initialize GTK. A display session is required");
    }
    gtkMain() = new Gtk::Main(argc, argv);
}

void runGtkLoop() {
    if (quitRequested()) {
        return;
    }
    Gtk::Main::run();
}

void requestGtkQuit() {
    const bool alreadyRequested = quitRequested();
    quitRequested() = true;

    // quit() ends only the innermost loop. A file dialog runs its own loop, so
    // an idle callback then ends the outer GuiModule::run loop as well.
    const guint level = Gtk::Main::level();
    if (level == 0) {
        return;
    }

    Gtk::Main::quit();
    if (!alreadyRequested && level > 1) {
        Glib::signal_idle().connect([]() -> bool {
            if (Gtk::Main::level() > 0) {
                Gtk::Main::quit();
            }
            return false;
        });
    }
}

} // namespace Luwow::Gui

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
