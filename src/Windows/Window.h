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
    uint16_t registerControl(IControl* control);
    void unregisterControl(uint16_t id);
    IControl* getControl(uint16_t id) const;

    // The window was destroyed, by close() or the user, along with its child controls and menu bar
    void destroyed();
    // The window was moved or resized
    void boundsChanged() {
        if (onBoundsChanged) onBoundsChanged();
    }

    HWND getHWND() const { return hWnd; };

private:
    uint16_t nextControlId = 1;
    std::unordered_map<uint16_t, IControl*> controls;
    HWND hWnd;
    // Shown and not yet destroyed, one of the windows the message loop runs for
    bool shown = false;
};

// The main loop's count of shown windows. Closing the last one ends the loop, and a window that's never
// shown doesn't keep it running.
void windowShown();
void windowClosed();

} // namespace Luwow::Gui
