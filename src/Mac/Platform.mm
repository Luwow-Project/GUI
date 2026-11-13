#import <Cocoa/Cocoa.h>

#include "GuiModule.h"

namespace Luwow::Gui {

// stop: takes effect once an event is handled, so an empty one is posted for it
void quitMainLoop() {
    [NSApp stop:nil];
    NSEvent* wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint modifierFlags:0
        timestamp:0 windowNumber:0 context:nil subtype:0 data1:0 data2:0];
    [NSApp postEvent:wake atStart:YES];
}

void GuiModule::startPlatform() {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
    }
}

bool GuiModule::post(const std::shared_ptr<MainThreadCall>& call) {
    std::shared_ptr<MainThreadCall> held = call;
    dispatch_async(dispatch_get_main_queue(), ^{
        held->run();
    });
    return true;
}

void GuiModule::runLoop() {
    @autoreleasepool {
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
}

} // namespace Luwow::Gui
