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
// Each interface is a Control's native control on the platform. Destroying one takes it off the screen,
// and a Control destroys its children's native controls before its own.
namespace Luwow::Gui {

// The topics other libraries request the gui library's services on, through the host
namespace Topics {
    // Replies with the native handle of the control at the top of message.state's stack, getNativeHandle's, as
    // "<Type> <address>" in message.data, the address in decimal and 0 while the control is detached or its
    // window is closed. Result is 1 for a control, and stays 0 for anything else.
    inline constexpr const char* NativeHandle = "gui-native-handle";
    // Runs a function on the main thread, where the native controls are used, and waits for it. message.data is
    // "<function> <context>", the decimal addresses of a void (*)(void*) and what it's called with. Result is 1
    // once it ran, and stays 0 when the main loop has ended.
    inline constexpr const char* RunOnMainThread = "gui-run-on-main-thread";
}

class IControl {
public:
    virtual ~IControl() = default;
    // The platform's object of the native control, for other libraries to build on.
    virtual void* getNativeHandle() const = 0;
};

// A control the user activates, such as a button or a menu item, which runs onCommand
class ICommandControl : public IControl {
public:
    std::function<void()> onCommand;
};

// A text field's events, which run on the main thread
class ITextEditorControl : public IControl {
public:
    virtual ~ITextEditorControl() = default;
    // The user changed the text
    std::function<void()> onTextChanged;
    // The text scrolled, by the user or as the text changed. Where the platform doesn't report it, it doesn't run:
    // a TextBox on Windows and macOS.
    std::function<void()> onScrolled;
};

class IWindow : public IControl {
public:
    virtual ~IWindow() = default;
    // Runs on the main thread when the window is moved or resized, by the user or by setBounds
    std::function<void()> onBoundsChanged;

    virtual void show() = 0;
    virtual void close() = 0;
    // Whether the window is still on screen, false once it's closed
    virtual bool isOpen() const = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
    virtual void getBounds(int& left, int& top, int& width, int& height) const = 0;
    // nullptr for an id that's no longer registered
};

class IButton : public ICommandControl {
public:
    virtual ~IButton() = default;
    virtual void setCaption(const std::string& caption) = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

class ITextLabel : public IControl {
public:
    virtual ~ITextLabel() = default;
    virtual void setText(const std::string& text) = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

// A single-line text field
class ITextBox : public ITextEditorControl {
public:
    virtual ~ITextBox() = default;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

// A multi-line text field
class ITextArea : public ITextEditorControl {
public:
    virtual ~ITextArea() = default;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

// A list of items the user picks one of. Index is 0-based, -1 for no item. onSelectionChanged runs on the main
// thread when the user picks one.
class IDropdown : public IControl {
public:
    virtual ~IDropdown() = default;
    std::function<void()> onSelectionChanged;
    virtual void setItems(const std::vector<std::string>& items) = 0;
    virtual void setSelected(int index) = 0;
    virtual int getSelected() const = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

// A check box, which the user toggles by activating it
class ICheckbox : public ICommandControl {
public:
    virtual ~ICheckbox() = default;
    virtual void setCaption(const std::string& caption) = 0;
    virtual void setChecked(bool checked) = 0;
    virtual bool isChecked() const = 0;
    virtual void setBounds(int left, int top, int width, int height) = 0;
};

class IMenuItem : public ICommandControl {
public:
    virtual ~IMenuItem() = default;
    virtual void setTitle(const std::string& title) = 0;
};

class IMenu : public IControl {
public:
    virtual ~IMenu() = default;
    virtual void setTitle(const std::string& title) = 0;
    virtual void insertItem(size_t index, IMenuItem* item) = 0;
};

class IMenuBar : public IControl {
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
    virtual ITextLabel* createTextLabel(const TextLabelDescriptor& descriptor, IWindow* window) = 0;
    virtual ITextBox* createTextBox(const TextBoxDescriptor& descriptor, IWindow* window) = 0;
    virtual ITextArea* createTextArea(const TextAreaDescriptor& descriptor, IWindow* window) = 0;
    virtual IDropdown* createDropdown(const DropdownDescriptor& descriptor, IWindow* window) = 0;
    virtual ICheckbox* createCheckbox(const CheckboxDescriptor& descriptor, IWindow* window) = 0;
    virtual IMenuBar* createMenuBar(IWindow* window) = 0;
    virtual IMenu* createMenu(const MenuDescriptor& descriptor) = 0;
    virtual IMenuItem* createMenuItem(const MenuItemDescriptor& descriptor, IWindow* window) = 0;
    virtual std::vector<std::string> showFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) = 0;
};

} // namespace Luwow::Gui
