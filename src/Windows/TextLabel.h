#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class TextLabel : public ITextLabel {
public:
    TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent);
    ~TextLabel();

    void setText(const std::string& text) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    HWND hLabel = nullptr;
};

} // namespace Luwow::Gui
