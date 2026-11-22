#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class Checkbox : public ICheckbox {
public:
    Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent);
    ~Checkbox();
    void* getNativeHandle() const override { return hCheck; }

    void setCaption(const std::string& caption) override;
    void setChecked(bool checked) override;
    bool isChecked() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    uint16_t id = 0;
    HWND hCheck = nullptr;
};

} // namespace Luwow::Gui
