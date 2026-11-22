#import <Cocoa/Cocoa.h>

#include "Checkbox.h"
#include "Window.h"
#include <stdexcept>

@interface LuwowCheckboxTarget : NSObject
@property (nonatomic, assign) Luwow::Gui::Checkbox* cppCheckbox;
@end

@implementation LuwowCheckboxTarget
- (void)toggled:(id)sender {
    (void)sender;
    if (self.cppCheckbox && self.cppCheckbox->onCommand) self.cppCheckbox->onCommand();
}
@end

namespace Luwow::Gui {

Checkbox::Checkbox(const CheckboxDescriptor& descriptor, IWindow* parent) {
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
        [button setButtonType:NSButtonTypeSwitch];
        [button setTitle:[NSString stringWithUTF8String:descriptor.Caption.c_str()]];
        [button setState:descriptor.Checked ? NSControlStateValueOn : NSControlStateValueOff];

        LuwowCheckboxTarget* target = [[LuwowCheckboxTarget alloc] init];
        target.cppCheckbox = this;
        [button setTarget:target];
        [button setAction:@selector(toggled:)];

        [contentView addSubview:button];

        nativeButton = (__bridge_retained void*)button;
        nativeTarget = (__bridge_retained void*)target;
    }
}

Checkbox::~Checkbox() {
    @autoreleasepool {
        if (nativeButton) {
            NSButton* button = (__bridge NSButton*)nativeButton;
            [button setTarget:nil];
            [button removeFromSuperview];
        }
        if (nativeTarget) {
            LuwowCheckboxTarget* target = (__bridge_transfer LuwowCheckboxTarget*)nativeTarget;
            nativeTarget = nullptr;
            target.cppCheckbox = nullptr;
        }
        if (nativeButton) {
            id button = (__bridge_transfer id)nativeButton;
            nativeButton = nullptr;
            (void)button;
        }
    }
}

void Checkbox::setCaption(const std::string& caption) {
    NSButton* button = (__bridge NSButton*)nativeButton;
    [button setTitle:[NSString stringWithUTF8String:caption.c_str()]];
}

void Checkbox::setChecked(bool checked) {
    NSButton* button = (__bridge NSButton*)nativeButton;
    [button setState:checked ? NSControlStateValueOn : NSControlStateValueOff];
}

bool Checkbox::isChecked() const {
    NSButton* button = (__bridge NSButton*)nativeButton;
    return [button state] == NSControlStateValueOn;
}

void Checkbox::setBounds(int left, int top, int width, int height) {
    NSButton* button = (__bridge NSButton*)nativeButton;
    [button setFrame:NSMakeRect(left, top, width, height)];
}

} // namespace Luwow::Gui
