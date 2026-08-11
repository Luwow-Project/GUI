#import <Cocoa/Cocoa.h>

#include "MenuItem.h"
#include "Window.h"
#include <stdexcept>
#include <string>
#include "lua.h"
#include "lualib.h"

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

MenuItem::MenuItem(const MenuItemDescriptor& descriptor, IWindow* parent)
    : descriptor(descriptor) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    (void)parentWindow->registerCommandControl(this);

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

void MenuItem::onCommand() {
    lua_State* L = descriptor.L;
    lua_getref(L, descriptor.OnSelectedRef);
    lua_pcall(L, 0, 0, 0);
}

} // namespace Luwow::Gui
