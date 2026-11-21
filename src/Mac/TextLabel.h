#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Window;

class TextLabel : public ITextLabel {
public:
    TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent);
    ~TextLabel() override;
    void* getNativeHandle() const override { return nativeLabel; }

    void setText(const std::string& text) override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    void* nativeLabel = nullptr;
};

} // namespace Luwow::Gui
