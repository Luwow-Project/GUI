#pragma once

#include "ILuauModule.h"
#include "Descriptors.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Luwow::Engine {
class ILuauHost;
}

// Abstract interface for the GUI API consistent across platform implementations.
// Each interface is a node's control on the platform. Destroying one takes it off the screen,
// and a node destroys its children's controls before its own.
namespace Luwow::Gui {

class ICommandControl {
public:
    virtual ~ICommandControl() = default;
    virtual void onCommand() = 0;
};

// A control the user activates, such as a button or a menu item, which runs onActivated
class IActivatable : public ICommandControl {
public:
    std::function<void()> onActivated;
    void onCommand() override {
        if (onActivated) onActivated();
    }
};

class IWindow {
public:
    virtual ~IWindow() = default;
    // Runs on the main thread when the window is moved or resized, by the user or by setBounds
    std::function<void()> onBoundsChanged;

    virtual void show() = 0;
    virtual void close() = 0;
    // Whether the window is still on screen, false once it's closed
    virtual bool isOpen() const = 0;
    // The platform's window, an HWND on Windows, an NSWindow on macOS and a GtkWindow on Linux, for other
    // libraries to draw into
    virtual void* getNativeHandle() const = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
    virtual void getBounds(int& left, int& top, int& width, int& height) const = 0;
    virtual uint16_t registerCommandControl(ICommandControl* commandControl) = 0;
    virtual void unregisterCommandControl(uint16_t id) = 0;
    // nullptr for an id that's no longer registered
    virtual ICommandControl* getCommandControl(uint16_t id) const = 0;
};

class IButton : public IActivatable {
public:
    virtual ~IButton() = default;
    virtual void setCaption(const std::string& caption) = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

class IMenuItem : public IActivatable {
public:
    virtual ~IMenuItem() = default;
    virtual void setTitle(const std::string& title) = 0;
};

class IMenu {
public:
    virtual ~IMenu() = default;
    virtual void setTitle(const std::string& title) = 0;
    virtual void insertItem(size_t index, IMenuItem* item) = 0;
};

class IMenuBar {
public:
    virtual ~IMenuBar() = default;
    virtual void insertMenu(size_t index, IMenu* menu) = 0;
};

// Interface for the GUI API consistent across platforms
class IGuiModule : public Luwow::Engine::ILuauModule {
public:
    virtual ~IGuiModule() = default;
    virtual Luwow::Engine::ILuauHost* getHost() const = 0;
    virtual bool isMainThread() const = 0;
    // Runs work on the main thread and waits for it, releasing the state lock meanwhile. Returns false,
    // without running it, when the main thread's message loop has ended or won't run.
    virtual bool runOnMainThread(const std::function<void()>& work) = 0;

    virtual IWindow* createWindow(const WindowDescriptor& descriptor) = 0;
    virtual IButton* createButton(const ButtonDescriptor& descriptor, IWindow* window) = 0;
    virtual IMenuBar* createMenuBar(IWindow* window) = 0;
    virtual IMenu* createMenu(const MenuDescriptor& descriptor) = 0;
    virtual IMenuItem* createMenuItem(const MenuItemDescriptor& descriptor, IWindow* window) = 0;
    virtual std::vector<std::string> showFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) = 0;
};

} // namespace Luwow::Gui
