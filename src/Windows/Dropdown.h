#pragma once

#include "IGuiModule.h"
#include <windows.h>

namespace Luwow::Gui {

class Window;

class Dropdown : public IDropdown {
public:
    Dropdown(const DropdownDescriptor& descriptor, IWindow* parent);
    ~Dropdown();
    void* getNativeHandle() const override { return hCombo; }

    void setItems(const std::vector<std::string>& items) override;
    void setSelected(int index) override;
    int getSelected() const override;
    void setBounds(int left, int top, int width, int height) override;

private:
    Window* window = nullptr;
    uint16_t id = 0;
    HWND hCombo = nullptr;
};

} // namespace Luwow::Gui
