#import <Cocoa/Cocoa.h>

#include "Window.h"
#include <stdexcept>
#include <string>
#include "lua.h"
#include "lualib.h"

@interface LuwowFlippedContentView : NSView
@end

@implementation LuwowFlippedContentView
- (BOOL)isFlipped {
    return YES;
}
@end

@interface LuwowWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) Luwow::Gui::Window* owner;
@end

@implementation LuwowWindowDelegate
- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    if (self.owner) self.owner->willClose();
}

- (void)windowDidMove:(NSNotification*)notification {
    (void)notification;
    if (self.owner) self.owner->boundsChanged();
}

- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    if (self.owner) self.owner->boundsChanged();
}
@end

namespace Luwow::Gui {

Window::Window(const WindowDescriptor& descriptor) {
    @autoreleasepool {
        NSScreen* screen = [NSScreen mainScreen];
        NSRect screenFrame = [screen frame];
        CGFloat y = NSHeight(screenFrame) - descriptor.Top - descriptor.Height;
        NSRect frame = NSMakeRect(descriptor.Left, y, descriptor.Width, descriptor.Height);

        NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
            | NSWindowStyleMaskResizable;
        NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];
        [window setTitle:[NSString stringWithUTF8String:descriptor.Title.c_str()]];
        [window setReleasedWhenClosed:NO];

        NSRect contentBounds = [[window contentView] bounds];
        LuwowFlippedContentView* cv = [[LuwowFlippedContentView alloc] initWithFrame:contentBounds];
        [window setContentView:cv];

        LuwowWindowDelegate* delegate = [[LuwowWindowDelegate alloc] init];
        delegate.owner = this;
        [window setDelegate:delegate];

        nativeWindow = (__bridge_retained void*)window;
        nativeDelegate = (__bridge_retained void*)delegate;
    }
}

Window::~Window() {
    @autoreleasepool {
        if (nativeDelegate) {
            ((__bridge LuwowWindowDelegate*)nativeDelegate).owner = nullptr;
        }
        if (nativeWindow && open) {
            [(__bridge NSWindow*)nativeWindow close];
            willClose();
        }
        if (nativeDelegate) {
            id delegate = (__bridge_transfer id)nativeDelegate;
            nativeDelegate = nullptr;
            (void)delegate;
        }
        if (nativeWindow) {
            id window = (__bridge_transfer id)nativeWindow;
            nativeWindow = nullptr;
            (void)window;
        }
    }
}

uint16_t Window::registerCommandControl(ICommandControl* commandControl) {
    uint16_t id = nextCommandId++;
    commandControls[id] = commandControl;
    return id;
}

void Window::unregisterCommandControl(uint16_t id) {
    commandControls.erase(id);
}

ICommandControl* Window::getCommandControl(uint16_t id) const {
    auto it = commandControls.find(id);
    return it == commandControls.end() ? nullptr : it->second;
}

void Window::show() {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w) {
        throw std::runtime_error("Window not found");
    }
    if (!open) {
        return;
    }
    if (!shown) {
        shown = true;
        windowShown();
    }
    [w makeKeyAndOrderFront:nil];
}

void Window::willClose() {
    open = false;
    if (shown) {
        shown = false;
        windowClosed();
    }
}

void Window::close() {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w || !open) {
        return;
    }
    [w close];
    willClose();
}

void Window::setTitle(const std::string& title) {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w || !open) {
        return;
    }
    [w setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

// Left and Top are from the screen's top left, as on Windows, and the size is the content's
void Window::setBounds(int left, int top, int width, int height) {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w || !open) {
        return;
    }
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    CGFloat y = NSHeight(screenFrame) - top - height;
    NSRect content = NSMakeRect(left, y, width, height);
    [w setFrame:[w frameRectForContentRect:content] display:YES];
}

// As setBounds takes them: the content's size, from the screen's top left
void Window::getBounds(int& left, int& top, int& width, int& height) const {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w || !open) {
        return;
    }
    NSRect content = [w contentRectForFrameRect:[w frame]];
    NSRect screenFrame = [[NSScreen mainScreen] frame];
    left = (int)NSMinX(content);
    width = (int)NSWidth(content);
    height = (int)NSHeight(content);
    top = (int)(NSHeight(screenFrame) - NSMaxY(content));
}

void* Window::getNativeContentView() const {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w) {
        return nullptr;
    }
    return (__bridge void*)[w contentView];
}

} // namespace Luwow::Gui
