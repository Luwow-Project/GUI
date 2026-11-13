#include "GuiModule.h"

#include <glib.h>
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
Gtk::Main* gtkMain = nullptr;

// A call another thread posted, run by the main loop
gboolean runMainThreadCall(gpointer data) {
    auto* call = static_cast<std::shared_ptr<MainThreadCall>*>(data);
    (*call)->run();
    delete call;
    return G_SOURCE_REMOVE;
}

} // namespace

// Ends the loop, including when a modal file dialog runs its own loop inside it. quit() ends only the
// innermost loop, so an idle callback ends each outer one in turn.
void quitMainLoop() {
    if (Gtk::Main::level() == 0) {
        return;
    }
    Gtk::Main::quit();
    if (Gtk::Main::level() > 1) {
        Glib::signal_idle().connect([]() -> bool {
            const guint level = Gtk::Main::level();
            if (level == 0) {
                return false;
            }
            Gtk::Main::quit();
            return level > 1;
        });
    }
}

void GuiModule::startPlatform() {
    if (gtkMain) {
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
    gtkMain = new Gtk::Main(argc, argv);
}

bool GuiModule::post(const std::shared_ptr<MainThreadCall>& call) {
    g_idle_add(runMainThreadCall, new std::shared_ptr<MainThreadCall>(call));
    return true;
}

void GuiModule::runLoop() {
    Gtk::Main::run();
}

} // namespace Luwow::Gui

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
