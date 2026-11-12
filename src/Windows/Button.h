#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class Button : public IButton {
public:
    Button(const ButtonDescriptor& descriptor, IWindow* parent);
    ~Button();

    void setCaption(const std::string& caption) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    uint16_t id = 0;
    HWND hButton = nullptr;
};

} // namespace Luwow::Gui
