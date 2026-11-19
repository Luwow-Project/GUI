#import <Cocoa/Cocoa.h>

#include "TextArea.h"
#include "Window.h"
#include <stdexcept>

@interface LuwowTextAreaObserver : NSObject
@property (nonatomic, assign) Luwow::Gui::TextArea* cppTextArea;
@end

@implementation LuwowTextAreaObserver
- (void)textDidChange:(NSNotification*)notification {
    (void)notification;
    if (self.cppTextArea) {
        self.cppTextArea->textChanged();
    }
}
@end

namespace Luwow::Gui {

TextArea::TextArea(const TextAreaDescriptor& descriptor, IWindow* parent) {
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
        NSScrollView* scroll = [[NSScrollView alloc] initWithFrame:frame];
        [scroll setHasVerticalScroller:YES];
        [scroll setHasHorizontalScroller:NO];
        [scroll setAutohidesScrollers:YES];
        [scroll setBorderType:NSBezelBorder];

        NSTextView* textView = [[NSTextView alloc] initWithFrame:[[scroll contentView] bounds]];
        [textView setString:[NSString stringWithUTF8String:descriptor.Text.c_str()]];
        [textView setEditable:YES];
        [textView setSelectable:YES];
        [textView setRichText:NO];
        [textView setAllowsUndo:YES];
        [textView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [scroll setDocumentView:textView];

        LuwowTextAreaObserver* observer = [[LuwowTextAreaObserver alloc] init];
        observer.cppTextArea = this;
        [[NSNotificationCenter defaultCenter] addObserver:observer
                                                 selector:@selector(textDidChange:)
                                                     name:NSTextDidChangeNotification
                                                   object:textView];

        [contentView addSubview:scroll];

        nativeScroll = (__bridge_retained void*)scroll;
        nativeTextView = (__bridge_retained void*)textView;
        nativeObserver = (__bridge_retained void*)observer;
    }
}

TextArea::~TextArea() {
    @autoreleasepool {
        if (nativeObserver) {
            LuwowTextAreaObserver* observer = (__bridge LuwowTextAreaObserver*)nativeObserver;
            [[NSNotificationCenter defaultCenter] removeObserver:observer];
            observer.cppTextArea = nullptr;
        }
        if (nativeScroll) {
            NSScrollView* scroll = (__bridge NSScrollView*)nativeScroll;
            [scroll removeFromSuperview];
        }
        if (nativeObserver) {
            id observer = (__bridge_transfer id)nativeObserver;
            nativeObserver = nullptr;
            (void)observer;
        }
        if (nativeTextView) {
            id textView = (__bridge_transfer id)nativeTextView;
            nativeTextView = nullptr;
            (void)textView;
        }
        if (nativeScroll) {
            id scroll = (__bridge_transfer id)nativeScroll;
            nativeScroll = nullptr;
            (void)scroll;
        }
    }
}

void TextArea::setText(const std::string& text) {
    NSTextView* textView = (__bridge NSTextView*)nativeTextView;
    updating = true;
    [textView setString:[NSString stringWithUTF8String:text.c_str()]];
    updating = false;
}

std::string TextArea::getText() const {
    NSTextView* textView = (__bridge NSTextView*)nativeTextView;
    NSString* value = [textView string];
    if (!value) return {};
    const char* utf8 = [value UTF8String];
    return utf8 ? std::string(utf8) : std::string();
}

void TextArea::setBounds(int left, int top, int width, int height) {
    NSScrollView* scroll = (__bridge NSScrollView*)nativeScroll;
    [scroll setFrame:NSMakeRect(left, top, width, height)];
}

void TextArea::textChanged() {
    if (!updating && onTextChanged) onTextChanged();
}

} // namespace Luwow::Gui
