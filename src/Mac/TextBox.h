#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Window;

class TextBox : public ITextBox {
public:
    TextBox(const TextBoxDescriptor& descriptor, IWindow* parent);
    ~TextBox() override;
    void* getNativeHandle() const override { return nativeField; }

    void setText(const std::string& text) override;
    std::string getText() const override;
    void setBounds(int left, int top, int width, int height) override;

    void textChanged();

private:
    Window* window = nullptr;
    void* nativeField = nullptr;
    void* nativeDelegate = nullptr;
    bool updating = false;
};

} // namespace Luwow::Gui
