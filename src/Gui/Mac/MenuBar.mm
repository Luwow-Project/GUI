#import <Cocoa/Cocoa.h>

#include "MenuBar.h"
#include "Window.h"
#include <stdexcept>
#include <string>

namespace Luwow::Gui {

// Strip Windows-style mnemonics ("&File" → "File", "&&" → "&").
static std::string stripMnemonic(const std::string& title) {
    std::string result;
    result.reserve(title.size());
    for (size_t i = 0; i < title.size(); ++i) {
        if (title[i] == '&' && i + 1 < title.size()) {
            result.push_back(title[++i]);
            continue;
        }
        if (title[i] != '&') {
            result.push_back(title[i]);
        }
    }
    return result;
}

MenuBar::MenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    @autoreleasepool {
        NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@""];

        // Cocoa expects the first menu to be the application menu; its title is
        // replaced with the process name. Provide a minimal Quit item so Cmd+Q works.
        NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
        NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@""];
        NSString* appName = [[NSProcessInfo processInfo] processName];
        NSMenuItem* quitItem = [[NSMenuItem alloc]
            initWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                   action:@selector(terminate:)
            keyEquivalent:@"q"];
        [appMenu addItem:quitItem];
        [appMenuItem setSubmenu:appMenu];
        [mainMenu addItem:appMenuItem];

        for (const MenuDescriptor& menuDescriptor : descriptor.Menus) {
            std::string menuTitle = stripMnemonic(menuDescriptor.Title);
            NSMenu* submenu = [[NSMenu alloc] initWithTitle:[NSString stringWithUTF8String:menuTitle.c_str()]];

            for (const MenuItemDescriptor& itemDescriptor : menuDescriptor.Items) {
                auto item = std::make_unique<MenuItem>(itemDescriptor, parentWindow);
                NSMenuItem* nativeItem = (__bridge NSMenuItem*)item->getNativeMenuItem();
                [submenu addItem:nativeItem];
                items.push_back(std::move(item));
            }

            NSMenuItem* topItem = [[NSMenuItem alloc] init];
            [topItem setTitle:[NSString stringWithUTF8String:menuTitle.c_str()]];
            [topItem setSubmenu:submenu];
            [mainMenu addItem:topItem];
        }

        [NSApp setMainMenu:mainMenu];
        nativeMainMenu = (__bridge_retained void*)mainMenu;
    }
}

MenuBar::~MenuBar() {
    @autoreleasepool {
        // Drop C++ item ownership first while the NSMenu still retains the
        // NSMenuItems, then release the menu tree.
        items.clear();
        if (nativeMainMenu) {
            if ([NSApp mainMenu] == (__bridge NSMenu*)nativeMainMenu) {
                [NSApp setMainMenu:nil];
            }
            id menu = (__bridge_transfer id)nativeMainMenu;
            nativeMainMenu = nullptr;
            (void)menu;
        }
    }
}

void getMenuBarTable(lua_State* L, MenuBar* menuBar) {
    lua_createtable(L, 0, 0);
    lua_pushlightuserdata(L, menuBar);
    lua_setfield(L, -2, "menuBar");
    lua_setreadonly(L, -1, 1);
}

} // namespace Luwow::Gui
