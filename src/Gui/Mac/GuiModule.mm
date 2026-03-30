#import <Cocoa/Cocoa.h>

#include "GuiModule.h"
#include "Engine.h"
#include "Window.h"
#include "Button.h"

#include "lua.h"
#include "lualib.h"
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

static int createWindow(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    WindowDescriptor windowDescriptor = getWindowDescriptor(L);
    Window* window = static_cast<Window*>(gui->createWindow(windowDescriptor));
    getWindowTable(L, window);

    lua_setreadonly(L, -1, 0);
    lua_pushlightuserdata(L, window);
    lua_setfield(L, -2, "__window_ptr");
    lua_setreadonly(L, -1, 1);
    return 1;
}

static int createButton(lua_State* L) {
    GuiModule* gui = getModuleInstance(L);
    ButtonDescriptor buttonDescriptor = getButtonDescriptor(L);

    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "__window_ptr");
    Window* parent = static_cast<Window*>(lua_touserdata(L, -1));
    if (!parent) {
        luaL_error(L, "Parent window not found creating button.");
    }
    lua_pop(L, 1);

    Button* button = static_cast<Button*>(gui->createButton(buttonDescriptor, parent));
    getButtonTable(L, button);
    return 1;
}

const char* GuiModule::getModuleName() const {
    return "gui.luau";
}

static LuauExport exports[] = {
    { "createWindow", createWindow },
    { "createButton", createButton },
    { nullptr, nullptr }
};

const LuauExport* GuiModule::getExports() const {
    return exports;
}

} // namespace Luwow::Gui
