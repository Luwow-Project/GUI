#include "MenuItem.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

MenuItem::MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent) : title(descriptor.Title) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    id = window->registerCommandControl(this);
}

MenuItem::~MenuItem() {
    window->unregisterCommandControl(id);
    if (owner && window->isOpen()) DeleteMenu(owner, id, MF_BYCOMMAND);
}

void MenuItem::setTitle(const std::string& title) {
    this->title = title;
    if (owner && window->isOpen()) ModifyMenuA(owner, id, MF_BYCOMMAND | MF_STRING, id, title.c_str());
}

} // namespace Luwow::Gui
