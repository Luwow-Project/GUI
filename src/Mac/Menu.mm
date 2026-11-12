#import <Cocoa/Cocoa.h>

#include "Menu.h"
#include "MenuItem.h"
#include "Mnemonic.h"
#include <string>

namespace Luwow::Gui {

Menu::Menu(const MenuDescriptor& descriptor) {
    @autoreleasepool {
        NSString* title = [NSString stringWithUTF8String:stripMnemonic(descriptor.Title).c_str()];
        NSMenu* submenu = [[NSMenu alloc] initWithTitle:title];
        NSMenuItem* topItem = [[NSMenuItem alloc] init];
        [topItem setTitle:title];
        [topItem setSubmenu:submenu];

        nativeSubmenu = (__bridge_retained void*)submenu;
        nativeTopItem = (__bridge_retained void*)topItem;
    }
}

Menu::~Menu() {
    @autoreleasepool {
        if (nativeTopItem) {
            NSMenuItem* topItem = (__bridge NSMenuItem*)nativeTopItem;
            if ([topItem menu]) {
                [[topItem menu] removeItem:topItem];
            }
            id released = (__bridge_transfer id)nativeTopItem;
            nativeTopItem = nullptr;
            (void)released;
        }
        if (nativeSubmenu) {
            id released = (__bridge_transfer id)nativeSubmenu;
            nativeSubmenu = nullptr;
            (void)released;
        }
    }
}

void Menu::setTitle(const std::string& title) {
    NSString* text = [NSString stringWithUTF8String:stripMnemonic(title).c_str()];
    [(__bridge NSMenu*)nativeSubmenu setTitle:text];
    [(__bridge NSMenuItem*)nativeTopItem setTitle:text];
}

void Menu::insertItem(size_t index, IMenuItem* item) {
    NSMenu* submenu = (__bridge NSMenu*)nativeSubmenu;
    NSMenuItem* nativeItem = (__bridge NSMenuItem*)static_cast<MenuItem*>(item)->getNativeMenuItem();
    [submenu insertItem:nativeItem atIndex:(NSInteger)index];
}

} // namespace Luwow::Gui
