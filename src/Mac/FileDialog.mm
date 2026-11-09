#import <Cocoa/Cocoa.h>

#include "FileDialog.h"

#include <string>
#include <vector>

namespace Luwow::Gui {
namespace {

void ApplyCommonPanelSettings(NSSavePanel* panel, const FileDialogDescriptor& descriptor) {
    if (!descriptor.Title.empty()) {
        panel.title = [NSString stringWithUTF8String:descriptor.Title.c_str()];
        panel.message = panel.title;
    }

    if (!descriptor.InitialDirectory.empty()) {
        NSURL* directory = [NSURL fileURLWithPath:[NSString stringWithUTF8String:descriptor.InitialDirectory.c_str()]
                                      isDirectory:YES];
        if (directory) {
            panel.directoryURL = directory;
        }
    }

    if (!descriptor.DefaultFileName.empty()) {
        panel.nameFieldStringValue = [NSString stringWithUTF8String:descriptor.DefaultFileName.c_str()];
    }

    if (descriptor.Mode != FileDialogMode::OpenFolder && !descriptor.Filters.empty()) {
        NSMutableArray<NSString*>* extensions = [NSMutableArray array];
        bool allowAll = false;
        for (const FileDialogFilter& filter : descriptor.Filters) {
            for (const std::string& extension : filter.Extensions) {
                if (extension == "*") {
                    allowAll = true;
                    break;
                }
                [extensions addObject:[NSString stringWithUTF8String:extension.c_str()]];
            }
            if (allowAll) {
                break;
            }
        }
        if (!allowAll && extensions.count > 0) {
            panel.allowedFileTypes = extensions;
        }
    }
}

NSModalResponse RunPanel(NSSavePanel* panel, IWindow* parent) {
    NSWindow* owner = parent ? (__bridge NSWindow*)parent->getNativeHandle() : nil;
    if (!owner) {
        return [panel runModal];
    }

    __block NSModalResponse sheetResponse = NSModalResponseCancel;
    [panel beginSheetModalForWindow:owner completionHandler:^(NSModalResponse result) {
        sheetResponse = result;
        [NSApp stopModal];
    }];
    [NSApp runModalForWindow:owner];
    return sheetResponse;
}

} // namespace

std::vector<std::string> showNativeFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) {
    std::vector<std::string> paths;

    @autoreleasepool {
        if (descriptor.Mode == FileDialogMode::SaveFile) {
            NSSavePanel* panel = [NSSavePanel savePanel];
            panel.canCreateDirectories = YES;
            panel.showsTagField = NO;
            ApplyCommonPanelSettings(panel, descriptor);

            if (RunPanel(panel, parent) == NSModalResponseOK) {
                const char* path = panel.URL.path.UTF8String;
                if (path) {
                    paths.emplace_back(path);
                }
            }
        } else {
            NSOpenPanel* panel = [NSOpenPanel openPanel];
            panel.canChooseFiles = descriptor.Mode != FileDialogMode::OpenFolder;
            panel.canChooseDirectories = descriptor.Mode == FileDialogMode::OpenFolder;
            panel.allowsMultipleSelection = descriptor.Mode == FileDialogMode::OpenFiles;
            panel.resolvesAliases = YES;
            panel.canCreateDirectories = NO;
            ApplyCommonPanelSettings(panel, descriptor);

            if (RunPanel(panel, parent) == NSModalResponseOK) {
                for (NSURL* url in panel.URLs) {
                    const char* path = url.path.UTF8String;
                    if (path) {
                        paths.emplace_back(path);
                    }
                }
            }
        }
    }

    return paths;
}

} // namespace Luwow::Gui
