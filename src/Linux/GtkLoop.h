#pragma once

namespace Luwow::Gui {

// gtk_init and a process-lifetime Gtk::Main. Widgets must be created after this.
void ensureGtkInitialized();

// Blocks until a window is closed. Returns immediately if one already closed.
void runGtkLoop();

// Ends runGtkLoop, including when a modal file dialog is inside a nested loop.
void requestGtkQuit();

} // namespace Luwow::Gui
