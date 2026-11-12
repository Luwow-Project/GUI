#pragma once

namespace Luwow::Gui {

// gtk_init and a process-lifetime Gtk::Main. Widgets must be created after this.
void ensureGtkInitialized();

// A window was shown, or a shown one closed or destroyed. Closing the last one ends the loop, and a
// window that's never shown doesn't keep it running.
void windowShown();
void windowClosed();

// Blocks until the shown windows are closed. Returns at once when none is shown.
void runGtkLoop();

} // namespace Luwow::Gui
