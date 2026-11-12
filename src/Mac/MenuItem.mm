#import <Cocoa/Cocoa.h>

#include "MenuItem.h"
#include "Mnemonic.h"
#include <string>

@interface LuwowMenuItemTarget : NSObject
@property (nonatomic, assign) Luwow::Gui::MenuItem* cppMenuItem;
@end

@implementation LuwowMenuItemTarget
- (void)selected:(id)sender {
    (void)sender;
    if (self.cppMenuItem) {
        self.cppMenuItem->onCommand();
    }
}
@end

namespace Luwow::Gui {

MenuItem::MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent) {
    (void)parent;
    @autoreleasepool {
        std::string title = stripMnemonic(descriptor.Title);
        NSMenuItem* item = [[NSMenuItem alloc]
            initWithTitle:[NSString stringWithUTF8String:title.c_str()]
                   action:@selector(selected:)
            keyEquivalent:@""];

        LuwowMenuItemTarget* target = [[LuwowMenuItemTarget alloc] init];
        target.cppMenuItem = this;
        [item setTarget:target];

        nativeMenuItem = (__bridge_retained void*)item;
        nativeTarget = (__bridge_retained void*)target;
    }
}

MenuItem::~MenuItem() {
    @autoreleasepool {
        if (nativeMenuItem) {
            NSMenuItem* item = (__bridge NSMenuItem*)nativeMenuItem;
            [item setTarget:nil];
            [item setAction:nil];
            if ([item menu]) {
                [[item menu] removeItem:item];
            }
        }
        if (nativeTarget) {
            LuwowMenuItemTarget* target = (__bridge_transfer LuwowMenuItemTarget*)nativeTarget;
            nativeTarget = nullptr;
            target.cppMenuItem = nullptr;
        }
        if (nativeMenuItem) {
            id item = (__bridge_transfer id)nativeMenuItem;
            nativeMenuItem = nullptr;
            (void)item;
        }
    }
}

void MenuItem::setTitle(const std::string& title) {
    NSMenuItem* item = (__bridge NSMenuItem*)nativeMenuItem;
    [item setTitle:[NSString stringWithUTF8String:stripMnemonic(title).c_str()]];
}

} // namespace Luwow::Gui
