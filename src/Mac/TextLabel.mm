#import <Cocoa/Cocoa.h>

#include "TextLabel.h"
#include "Window.h"
#include <stdexcept>

namespace Luwow::Gui {

TextLabel::TextLabel(const TextLabelDescriptor& descriptor, IWindow* parent) {
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
        NSTextField* label = [[NSTextField alloc] initWithFrame:frame];
        [label setStringValue:[NSString stringWithUTF8String:descriptor.Text.c_str()]];
        [label setEditable:NO];
        [label setSelectable:NO];
        [label setBezeled:NO];
        [label setDrawsBackground:NO];
        [label setBordered:NO];
        [contentView addSubview:label];
        nativeLabel = (__bridge_retained void*)label;
    }
}

TextLabel::~TextLabel() {
    @autoreleasepool {
        if (nativeLabel) {
            NSTextField* label = (__bridge_transfer NSTextField*)nativeLabel;
            nativeLabel = nullptr;
            [label removeFromSuperview];
        }
    }
}

void TextLabel::setText(const std::string& text) {
    NSTextField* label = (__bridge NSTextField*)nativeLabel;
    [label setStringValue:[NSString stringWithUTF8String:text.c_str()]];
}

void TextLabel::setBounds(int left, int top, int width, int height) {
    NSTextField* label = (__bridge NSTextField*)nativeLabel;
    [label setFrame:NSMakeRect(left, top, width, height)];
}

} // namespace Luwow::Gui
