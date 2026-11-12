#import <Cocoa/Cocoa.h>

#include "MenuBar.h"
#include "Menu.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

MenuBar::MenuBar(IWindow* parent) {
    if (!dynamic_cast<Window*>(parent)) {
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

        [NSApp setMainMenu:mainMenu];
        nativeMainMenu = (__bridge_retained void*)mainMenu;
    }
}

MenuBar::~MenuBar() {
    @autoreleasepool {
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

// The application menu comes first, so the bar's menus start after it
void MenuBar::insertMenu(size_t index, IMenu* menu) {
    NSMenu* mainMenu = (__bridge NSMenu*)nativeMainMenu;
    NSMenuItem* topItem = (__bridge NSMenuItem*)static_cast<Menu*>(menu)->getNativeTopItem();
    [mainMenu insertItem:topItem atIndex:(NSInteger)index + 1];
}

} // namespace Luwow::Gui
