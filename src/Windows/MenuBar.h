#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class MenuBar : public IMenuBar {
public:
    explicit MenuBar(IWindow* parent);
    ~MenuBar() override;

    void insertMenu(size_t index, IMenu* menu) override;

private:
    Window* window = nullptr;
    HMENU menu = nullptr;
};

} // namespace Luwow::Gui
