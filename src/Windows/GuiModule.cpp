#include "GuiModule.h"
#include "Engine.h"
#include "Window.h"
#include "Button.h"
#include "MenuBar.h"
#include "Handles.h"

#include "lua.h"
#include "lualib.h"
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

namespace Luwow::Gui {
using ILuauModule = Luwow::Engine::ILuauModule;
using Engine = Luwow::Engine::Engine;

// For all methods that require the Gui instance, we need to get it from the userdata.
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

static Window* GetWindow(HWND hWnd) {
    return (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
        {
            // If all windows opened are closed
            Window* gui = GetWindow(hWnd);
            if (!gui) {
                throw std::runtime_error("Window not found");
            }
            PostQuitMessage(0);
            break;
        }
        case WM_COMMAND:
        {
            Window* gui = GetWindow(hWnd);
            if (!gui) {
                throw std::runtime_error("Window not found");
            }
            uint16_t commandId = LOWORD(wParam);
            ICommandControl* commandControl = gui->getCommandControl(commandId);
            if (commandControl) {
                commandControl->onCommand();
            }
            break;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

/*static*/ void GuiModule::MessagePump() {
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void GuiModule::setEngine(Engine* engine) {
    this->engine = engine;

    WNDCLASSEXA wcex;
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "LuwowWindow";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExA(&wcex);
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
