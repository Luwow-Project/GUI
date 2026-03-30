#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Button : public IButton {
public:
    Button(const ButtonDescriptor& descriptor, IWindow* parent);
    ~Button() override;

    void onCommand() override;

private:
    ButtonDescriptor descriptor;
    void* nativeButton = nullptr;
    void* nativeTarget = nullptr;
};

void getButtonTable(lua_State* L, Button* button);

} // namespace Luwow::Gui
