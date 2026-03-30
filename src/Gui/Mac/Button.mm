#import <Cocoa/Cocoa.h>

#include "Button.h"
#include "Window.h"
#include <stdexcept>
#include "lua.h"
#include "lualib.h"

@interface LuwowButtonTarget : NSObject
@property (nonatomic, assign) Luwow::Gui::Button* cppButton;
@end

@implementation LuwowButtonTarget
- (void)clicked:(id)sender {
    (void)sender;
    if (self.cppButton) {
        self.cppButton->onCommand();
    }
}
@end

namespace Luwow::Gui {

Button::Button(const ButtonDescriptor& descriptor, IWindow* parent) : descriptor(descriptor) {
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow) {
        throw std::runtime_error("Parent window is not a Window");
    }

    (void)parentWindow->registerCommandControl(this);

    NSView* contentView = (__bridge NSView*)parentWindow->getNativeContentView();
    if (!contentView) {
        throw std::runtime_error("Parent window has no content view");
    }

    @autoreleasepool {
        NSRect frame = NSMakeRect(descriptor.Left, descriptor.Top, descriptor.Width, descriptor.Height);
        NSButton* button = [[NSButton alloc] initWithFrame:frame];
        [button setTitle:[NSString stringWithUTF8String:descriptor.Caption.c_str()]];
        [button setButtonType:NSButtonTypeMomentaryPushIn];
        [button setBezelStyle:NSBezelStyleRounded];

        LuwowButtonTarget* target = [[LuwowButtonTarget alloc] init];
        target.cppButton = this;
        [button setTarget:target];
        [button setAction:@selector(clicked:)];

        [contentView addSubview:button];

        nativeButton = (__bridge_retained void*)button;
        nativeTarget = (__bridge_retained void*)target;
    }
}

Button::~Button() {
    @autoreleasepool {
        if (nativeTarget) {
            id target = (__bridge_transfer id)nativeTarget;
            nativeTarget = nullptr;
            (void)target;
        }
        if (nativeButton) {
            id button = (__bridge_transfer id)nativeButton;
            nativeButton = nullptr;
            (void)button;
        }
    }
}

void Button::onCommand() {
    lua_State* L = descriptor.L;
    lua_getref(L, descriptor.OnPressedRef);
    lua_pcall(L, 0, 0, 0);
}

void getButtonTable(lua_State* L, Button* button) {
    lua_createtable(L, 0, 0);
    lua_pushlightuserdata(L, button);
    lua_setfield(L, -2, "button");
    lua_setreadonly(L, -1, 1);
}

} // namespace Luwow::Gui
