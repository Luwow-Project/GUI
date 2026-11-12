#import <Cocoa/Cocoa.h>

#include "Button.h"
#include "Window.h"
#include <stdexcept>

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

Button::Button(const ButtonDescriptor& descriptor, IWindow* parent) {
    window = dynamic_cast<Window*>(parent);
    if (!window) {
        throw std::runtime_error("Parent window is not a Window");
    }

    NSView* contentView = (__bridge NSView*)window->getNativeContentView();
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
        if (nativeButton) {
            NSButton* button = (__bridge NSButton*)nativeButton;
            [button setTarget:nil];
            [button removeFromSuperview];
        }
        if (nativeTarget) {
            LuwowButtonTarget* target = (__bridge_transfer LuwowButtonTarget*)nativeTarget;
            nativeTarget = nullptr;
            target.cppButton = nullptr;
        }
        if (nativeButton) {
            id button = (__bridge_transfer id)nativeButton;
            nativeButton = nullptr;
            (void)button;
        }
    }
}

void Button::setCaption(const std::string& caption) {
    NSButton* button = (__bridge NSButton*)nativeButton;
    [button setTitle:[NSString stringWithUTF8String:caption.c_str()]];
}

void Button::setBounds(int left, int top, int width, int height) {
    NSButton* button = (__bridge NSButton*)nativeButton;
    [button setFrame:NSMakeRect(left, top, width, height)];
}

} // namespace Luwow::Gui
