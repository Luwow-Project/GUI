#pragma once

#include "IGuiModule.h"

namespace Luwow::Gui {

class Window;

class Checkbox : public ICheckbox {
public:
    Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent);
    ~Checkbox() override;
    void* getNativeHandle() const override { return nativeButton; }

    void setCaption(const std::string& caption) override;
    void setChecked(bool checked) override;
    bool isChecked() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    void* nativeButton = nullptr;
    void* nativeTarget = nullptr;
};

} // namespace Luwow::Gui
