#import <Cocoa/Cocoa.h>

#include "TextBox.h"
#include "Window.h"
#include <stdexcept>

@interface LuwowTextBoxDelegate : NSObject <NSTextFieldDelegate>
@property (nonatomic, assign) Luwow::Gui::TextBox* cppTextBox;
@end

@implementation LuwowTextBoxDelegate
- (void)controlTextDidChange:(NSNotification*)notification {
    (void)notification;
    if (self.cppTextBox) {
        self.cppTextBox->textChanged();
    }
}
@end

namespace Luwow::Gui {

TextBox::TextBox(const TextBoxDescriptor& descriptor, IWindow* parent) {
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
        NSTextField* field = [[NSTextField alloc] initWithFrame:frame];
        [field setStringValue:[NSString stringWithUTF8String:descriptor.Text.c_str()]];
        [field setEditable:YES];
        [field setSelectable:YES];
        [field setBezeled:YES];
        [field setBordered:YES];

        LuwowTextBoxDelegate* delegate = [[LuwowTextBoxDelegate alloc] init];
        delegate.cppTextBox = this;
        [field setDelegate:delegate];

        [contentView addSubview:field];

        nativeField = (__bridge_retained void*)field;
        nativeDelegate = (__bridge_retained void*)delegate;
    }
}

TextBox::~TextBox() {
    @autoreleasepool {
        if (nativeField) {
            NSTextField* field = (__bridge NSTextField*)nativeField;
            [field setDelegate:nil];
            [field removeFromSuperview];
        }
        if (nativeDelegate) {
            LuwowTextBoxDelegate* delegate = (__bridge_transfer LuwowTextBoxDelegate*)nativeDelegate;
            nativeDelegate = nullptr;
            delegate.cppTextBox = nullptr;
        }
        if (nativeField) {
            id field = (__bridge_transfer id)nativeField;
            nativeField = nullptr;
            (void)field;
        }
    }
}

void TextBox::setText(const std::string& text) {
    NSTextField* field = (__bridge NSTextField*)nativeField;
    updating = true;
    [field setStringValue:[NSString stringWithUTF8String:text.c_str()]];
    updating = false;
}

std::string TextBox::getText() const {
    NSTextField* field = (__bridge NSTextField*)nativeField;
    NSString* value = [field stringValue];
    if (!value) return {};
    const char* utf8 = [value UTF8String];
    return utf8 ? std::string(utf8) : std::string();
}

void TextBox::setBounds(int left, int top, int width, int height) {
    NSTextField* field = (__bridge NSTextField*)nativeField;
    [field setFrame:NSMakeRect(left, top, width, height)];
}

void TextBox::textChanged() {
    if (!updating && onTextChanged) onTextChanged();
}

} // namespace Luwow::Gui
