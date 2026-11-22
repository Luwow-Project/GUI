#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class TextBox : public ITextBox {
public:
    TextBox(const TextBoxDescriptor& descriptor, IWindow* parent);
    ~TextBox();
    void* getNativeHandle() const override { return hEdit; }

    void setText(const std::string& text) override;
    std::string getText() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    uint16_t id = 0;
    HWND hEdit = nullptr;
    bool updating = false;
};

} // namespace Luwow::Gui
