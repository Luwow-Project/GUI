#import <Cocoa/Cocoa.h>

#include "Dropdown.h"
#include "Window.h"
#include <stdexcept>

@interface LuwowDropdownTarget : NSObject
@property (nonatomic, assign) Luwow::Gui::Dropdown* cppDropdown;
@end

@implementation LuwowDropdownTarget
- (void)selected:(id)sender {
    (void)sender;
    if (self.cppDropdown && self.cppDropdown->onSelectionChanged) self.cppDropdown->onSelectionChanged();
}
@end

namespace Luwow::Gui {

Dropdown::Dropdown(const DropdownDescriptor& descriptor, IWindow* parent) {
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
        NSPopUpButton* popUp = [[NSPopUpButton alloc] initWithFrame:frame pullsDown:NO];

        LuwowDropdownTarget* target = [[LuwowDropdownTarget alloc] init];
        target.cppDropdown = this;
        [popUp setTarget:target];
        [popUp setAction:@selector(selected:)];

        [contentView addSubview:popUp];

        nativePopUp = (__bridge_retained void*)popUp;
        nativeTarget = (__bridge_retained void*)target;
    }
    setItems(descriptor.Items);
    setSelected(descriptor.SelectedIndex - 1);
}

Dropdown::~Dropdown() {
    @autoreleasepool {
        if (nativePopUp) {
            NSPopUpButton* popUp = (__bridge NSPopUpButton*)nativePopUp;
            [popUp setTarget:nil];
            [popUp removeFromSuperview];
        }
        if (nativeTarget) {
            LuwowDropdownTarget* target = (__bridge_transfer LuwowDropdownTarget*)nativeTarget;
            nativeTarget = nullptr;
            target.cppDropdown = nullptr;
        }
        if (nativePopUp) {
            id popUp = (__bridge_transfer id)nativePopUp;
            nativePopUp = nullptr;
            (void)popUp;
        }
    }
}

void Dropdown::setItems(const std::vector<std::string>& items) {
    NSPopUpButton* popUp = (__bridge NSPopUpButton*)nativePopUp;
    [popUp removeAllItems];
    for (const std::string& item : items) {
        NSString* title = [NSString stringWithUTF8String:item.c_str()];
        [[popUp menu] addItemWithTitle:title action:nil keyEquivalent:@""];
    }
}

void Dropdown::setSelected(int index) {
    NSPopUpButton* popUp = (__bridge NSPopUpButton*)nativePopUp;
    [popUp selectItemAtIndex:index];
}

int Dropdown::getSelected() const {
    NSPopUpButton* popUp = (__bridge NSPopUpButton*)nativePopUp;
    return (int)[popUp indexOfSelectedItem];
}

void Dropdown::setBounds(int left, int top, int width, int height) {
    NSPopUpButton* popUp = (__bridge NSPopUpButton*)nativePopUp;
    [popUp setFrame:NSMakeRect(left, top, width, height)];
}

} // namespace Luwow::Gui
