#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class TextArea : public ITextArea {
public:
    TextArea(const TextAreaDescriptor& descriptor, IWindow* parent);
    ~TextArea();

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
