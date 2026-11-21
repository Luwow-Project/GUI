#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Window;

class TextArea : public ITextArea {
public:
    TextArea(const TextAreaDescriptor& descriptor, IWindow* parent);
    ~TextArea() override;
    void* getNativeHandle() const override { return nativeTextView; }

    void setText(const std::string& text) override;
    std::string getText() const override;
    void setBounds(int left, int top, int width, int height) override;

    void textChanged();

private:
    Window* window = nullptr;
    void* nativeScroll = nullptr;
    void* nativeTextView = nullptr;
    void* nativeObserver = nullptr;
    bool updating = false;
};

} // namespace Luwow::Gui
