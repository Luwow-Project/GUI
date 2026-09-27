#import <Cocoa/Cocoa.h>

#include "GuiModule.h"
#include "Engine.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Handles.h"

#include "lua.h"
#include "lualib.h"
#include <stdexcept>
#include <string>

namespace Luwow::Gui {
using ILuauModule = Luwow::Engine::ILuauModule;
using Engine = Luwow::Engine::Engine;

static GuiModule* getModuleInstance(lua_State* L) {
    GuiModule* gui = static_cast<GuiModule*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!gui) {
        throw std::runtime_error("Gui module not found");
    }
    return gui;
}

GuiModule::GuiModule() : engine(nullptr) {}

ILuauModule* GuiModule::initialize(Engine* engine) {
    GuiModule* gui = new GuiModule();
    gui->setEngine(engine);
    registerHandles(engine->getMainState());
    return gui;
}

void GuiModule::MessagePump() {
    @autoreleasepool {
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
}

void GuiModule::setEngine(Engine* engine) {
    this->engine = engine;

    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
    }

    engine->setMessagePumpCallback(MessagePump);
}

IWindow* GuiModule::createWindow(const WindowDescriptor& descriptor) {
    return new Window(descriptor);
}

IButton* GuiModule::createButton(const ButtonDescriptor& descriptor, IWindow* parent) {
    return new Button(descriptor, parent);
}

IMenuBar* GuiModule::createMenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) {
    return new MenuBar(descriptor, parent);
}

static int createWindow(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    WindowDescriptor windowDescriptor = getWindowDescriptor(L);
    LuauWindow::Push(L, gui->createWindow(windowDescriptor));
    return 1;
}

static int createButton(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    ButtonDescriptor buttonDescriptor = getButtonDescriptor(L);
    IWindow* parent = LuauWindow::Check(L, 2)->get();

    LuauButton::Push(L, gui->createButton(buttonDescriptor, parent));
    return 1;
}

static int createMenuBar(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    MenuBarDescriptor menuBarDescriptor = getMenuBarDescriptor(L);
    IWindow* parent = LuauWindow::Check(L, 2)->get();

    LuauMenuBar::Push(L, gui->createMenuBar(menuBarDescriptor, parent));
    return 1;
}

const char* GuiModule::getModuleName() const {
    return "gui";
}

const char* GuiModule::getModuleAlias() const {
    return "Luwow";
}

static LuauExport exports[] = {
    { "createWindow", createWindow },
    { "createButton", createButton },
    { "createMenuBar", createMenuBar },
    { nullptr, nullptr }
};

const LuauExport* GuiModule::getExports() const {
    return exports;
}

} // namespace Luwow::Gui
