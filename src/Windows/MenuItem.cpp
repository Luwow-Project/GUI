#include "MenuItem.h"
#include "Window.h"
#include "ILuauHost.h"
#include <stdexcept>
#include "lua.h"
#include "lualib.h"

namespace Luwow::Gui {

MenuItem::MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent)
    : descriptor(descriptor) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = parentWindow->registerCommandControl(this);
}

void MenuItem::onCommand() {
    Luwow::Engine::StateLock lock(descriptor.host);
    lua_State* L = descriptor.L;
    lua_getref(L, descriptor.OnSelectedRef);
    lua_pcall(L, 0, 0, 0);
}

} // namespace Luwow::Gui
