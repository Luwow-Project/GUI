#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

// A menu of a menu bar, a popup of items
class Menu : public IMenu {
public:
    explicit Menu(const MenuDescriptor& descriptor);
    ~Menu() override;

    void setTitle(const std::string& title) override;
    void insertItem(size_t index, IMenuItem* item) override;

    HMENU getPopup() const { return popup; }
    const std::string& getTitle() const { return title; }
    // The bar it was put in, and the bar's window
    void attach(HMENU bar, Window* window);

private:
    // The menu's position in its bar, -1 if it isn't there
    int positionInBar() const;

    HMENU popup = nullptr;
    std::string title;
    HMENU bar = nullptr;
    Window* window = nullptr;
};

} // namespace Luwow::Gui
