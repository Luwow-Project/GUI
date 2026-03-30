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
@end

@implementation LuwowWindowDelegate
- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    [[NSApplication sharedApplication] terminate:nil];
}
@end

namespace Luwow::Gui {

Window::Window(const WindowDescriptor& descriptor) : descriptor(descriptor) {
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
        [window setDelegate:delegate];

        nativeWindow = (__bridge_retained void*)window;
        nativeDelegate = (__bridge_retained void*)delegate;
    }
}

Window::~Window() {
    @autoreleasepool {
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

static int show(lua_State* L) {
    Window* window = static_cast<Window*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!window) {
        throw std::runtime_error("Window not found");
    }
    window->show();
    return 0;
}

uint16_t Window::registerCommandControl(ICommandControl* commandControl) {
    uint16_t id = nextCommandId++;
    commandControls[id] = commandControl;
    return id;
}

ICommandControl* Window::getCommandControl(uint16_t id) const {
    return commandControls.at(id);
}

void Window::show() {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w) {
        throw std::runtime_error("Window not found");
    }
    [w makeKeyAndOrderFront:nil];
}

void* Window::getNativeContentView() const {
    NSWindow* w = (__bridge NSWindow*)nativeWindow;
    if (!w) {
        return nullptr;
    }
    return (__bridge void*)[w contentView];
}

void getWindowTable(lua_State* L, Window* window) {
    lua_createtable(L, 0, 2);
    lua_pushlightuserdata(L, window);
    lua_pushcclosure(L, &show, "show", 1);
    lua_setfield(L, -2, "show");
    lua_setreadonly(L, -1, 1);
}

} // namespace Luwow::Gui
