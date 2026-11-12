#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class MenuItem : public IMenuItem {
public:
    MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent);
    ~MenuItem() override;

    void setTitle(const std::string& title) override;

    uint16_t getId() const { return id; }
    const std::string& getTitle() const { return title; }
    // The menu it was put in
    void attach(HMENU owner) { this->owner = owner; }

private:
    Window* window = nullptr;
    uint16_t id = 0;
    std::string title;
    HMENU owner = nullptr;
};

} // namespace Luwow::Gui
