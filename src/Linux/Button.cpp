#include "Button.h"
#include "Mnemonic.h"
#include "Window.h"
#include "ILuauHost.h"

#include <stdexcept>
#include "lua.h"
#include "lualib.h"

namespace Luwow::Gui {

Button::Button(const ButtonDescriptor& descriptor, IWindow* parent) : descriptor(descriptor) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    Gtk::Fixed* fixed = parentWindow->getFixed();
    if (!fixed) {
        throw std::runtime_error("Parent window has no content area");
    }

    (void)parentWindow->registerCommandControl(this);

    button = Gtk::manage(new Gtk::Button(windowsMnemonicToGtk(descriptor.Caption), true));
    button->set_size_request(descriptor.Width, descriptor.Height);
    fixed->put(*button, descriptor.Left, descriptor.Top);
    button->show();

    clicked = button->signal_clicked().connect(sigc::mem_fun(*this, &Button::onCommand));
}

Button::~Button() {
    clicked.disconnect();
}

void Button::onCommand() {
    Luwow::Engine::StateLock lock(descriptor.host);
    lua_State* L = descriptor.L;
    lua_getref(L, descriptor.OnPressedRef);
    lua_pcall(L, 0, 0, 0);
}

} // namespace Luwow::Gui
