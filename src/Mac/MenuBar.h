#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class MenuBar : public IMenuBar {
public:
    explicit MenuBar(IWindow* parent);
    ~MenuBar() override;

    void insertMenu(size_t index, IMenu* menu) override;

private:
    void* nativeMainMenu = nullptr;
};

} // namespace Luwow::Gui
