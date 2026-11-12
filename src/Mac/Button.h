#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Window;

class Button : public IButton {
public:
    Button(const ButtonDescriptor& descriptor, IWindow* parent);
    ~Button() override;

    void setCaption(const std::string& caption) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    void* nativeButton = nullptr;
    void* nativeTarget = nullptr;
};

} // namespace Luwow::Gui
