#pragma once

#include "IGuiModule.h"
#include <unordered_map>

namespace Luwow::Gui {

class Window : public IWindow {
public:
    Window(const WindowDescriptor& descriptor);
    ~Window() override;
    void show() override;
    void close() override;
    bool isOpen() const override { return open; }
    void* getNativeHandle() const override { return nativeWindow; }
    void setTitle(const std::string& title) override;
    void setBounds(int left, int top, int width, int height) override;
    void getBounds(int& left, int& top, int& width, int& height) const override;
    uint16_t registerCommandControl(ICommandControl* commandControl) override;
    void unregisterCommandControl(uint16_t id) override;
    ICommandControl* getCommandControl(uint16_t id) const override;

    void* getNativeContentView() const;

    // The window was moved or resized
    void boundsChanged() {
        if (onBoundsChanged) onBoundsChanged();
    }
    // The window is closing, by close() or the user
    void willClose();

private:
    uint16_t nextCommandId = 1;
    std::unordered_map<uint16_t, ICommandControl*> commandControls;
    bool open = true;
    // Shown and not yet closed, one of the windows the run loop runs for
    bool shown = false;
    void* nativeWindow = nullptr;
    void* nativeDelegate = nullptr;
};

// The run loop's count of shown windows, which ends the loop when the last one closes
void windowShown();
void windowClosed();

} // namespace Luwow::Gui
