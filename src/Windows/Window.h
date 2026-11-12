#pragma once

#include "IGuiModule.h"
#include <vector>
#include <windows.h>
#include <unordered_map>

namespace Luwow::Gui {

class Window : public IWindow {
public:
    Window(const WindowDescriptor& descriptor);
    ~Window();
    virtual void show() override;
    virtual void close() override;
    bool isOpen() const override { return hWnd != NULL; }
    void* getNativeHandle() const override { return hWnd; }
    void setTitle(const std::string& title) override;
    void setBounds(int left, int top, int width, int height) override;
    void getBounds(int& left, int& top, int& width, int& height) const override;
    virtual uint16_t registerCommandControl(ICommandControl* commandControl) override;
    void unregisterCommandControl(uint16_t id) override;
    virtual ICommandControl* getCommandControl(uint16_t id) const override;

    // The window was destroyed, by close() or the user, along with its child controls and menu bar
    void destroyed();
    // The window was moved or resized
    void boundsChanged() {
        if (onBoundsChanged) onBoundsChanged();
    }

    HWND getHWND() const { return hWnd; };

private:
    uint16_t nextCommandId = 1;
    std::unordered_map<uint16_t, ICommandControl*> commandControls;
    HWND hWnd;
    // Shown and not yet destroyed, one of the windows the message loop runs for
    bool shown = false;
};

// The message loop's count of shown windows, which ends the loop when the last one goes
void windowShown();
void windowClosed();

} // namespace Luwow::Gui
