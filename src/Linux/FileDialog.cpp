#include "FileDialog.h"
#include "Window.h"

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/filefilter.h>
#include <glibmm/convert.h>
#include <memory>
#include <stdexcept>

namespace Luwow::Gui {
namespace {

std::string pathToUtf8(const std::string& path) {
    try {
        return Glib::filename_to_utf8(path);
    } catch (const Glib::Error&) {
        return path;
    }
}

std::string filenameFromUtf8(const std::string& path) {
    try {
        return Glib::filename_from_utf8(path);
    } catch (const Glib::Error&) {
        return path;
    }
}

Glib::ustring dialogTitle(const FileDialogDescriptor& descriptor) {
    if (!descriptor.Title.empty()) {
        return descriptor.Title;
    }
    switch (descriptor.Mode) {
        case FileDialogMode::SaveFile:
            return "Save";
        case FileDialogMode::OpenFolder:
            return "Open Folder";
        default:
            return "Open";
    }
}

Gtk::FileChooserAction dialogAction(const FileDialogDescriptor& descriptor) {
    switch (descriptor.Mode) {
        case FileDialogMode::OpenFolder:
            return Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER;
        case FileDialogMode::SaveFile:
            return Gtk::FILE_CHOOSER_ACTION_SAVE;
        case FileDialogMode::OpenFile:
        case FileDialogMode::OpenFiles:
            return Gtk::FILE_CHOOSER_ACTION_OPEN;
    }
    return Gtk::FILE_CHOOSER_ACTION_OPEN;
}

const char* acceptLabel(const FileDialogDescriptor& descriptor) {
    switch (descriptor.Mode) {
        case FileDialogMode::SaveFile:
            return "_Save";
        case FileDialogMode::OpenFolder:
            return "_Select";
        default:
            return "_Open";
    }
}

struct AddedFilter {
    Glib::RefPtr<Gtk::FileFilter> filter;
    std::string extension;
};

std::vector<AddedFilter> addFilters(Gtk::FileChooser& chooser, const FileDialogDescriptor& descriptor) {
    std::vector<AddedFilter> added;
    if (descriptor.Mode == FileDialogMode::OpenFolder || descriptor.Filters.empty()) {
        return added;
    }

    for (const FileDialogFilter& filterDesc : descriptor.Filters) {
        auto filter = Gtk::FileFilter::create();
        filter->set_name(filterDesc.Name);
        std::string extension;
        for (const std::string& ext : filterDesc.Extensions) {
            if (ext == "*") {
                filter->add_pattern("*");
                if (extension.empty()) {
                    extension = "*";
                }
            } else {
                filter->add_pattern("*." + ext);
                if (extension.empty()) {
                    extension = ext;
                }
            }
        }
        chooser.add_filter(filter);
        added.push_back({ filter, std::move(extension) });
    }

    if (!added.empty()) {
        chooser.set_filter(added.front().filter);
    }
    return added;
}

std::string extensionForFilter(const std::vector<AddedFilter>& added, const Glib::RefPtr<Gtk::FileFilter>& selected) {
    for (const AddedFilter& filter : added) {
        if (filter.filter == selected) {
            return filter.extension;
        }
    }
    return added.empty() ? std::string() : added.front().extension;
}

std::string withDefaultExtension(std::string path, const std::string& extension) {
    if (extension.empty() || extension == "*") {
        return path;
    }
    const auto slash = path.find_last_of('/');
    const std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    const auto dot = name.find_last_of('.');
    if (dot != std::string::npos && dot != 0) {
        return path;
    }
    path.push_back('.');
    path += extension;
    return path;
}

void applyCommonSettings(Gtk::FileChooser& chooser, const FileDialogDescriptor& descriptor) {
    if (!descriptor.InitialDirectory.empty()) {
        (void)chooser.set_current_folder(filenameFromUtf8(descriptor.InitialDirectory));
    }
    // GTK only honors the suggested name on save dialogs.
    if (descriptor.Mode == FileDialogMode::SaveFile && !descriptor.DefaultFileName.empty()) {
        chooser.set_current_name(descriptor.DefaultFileName);
    }
}

Gtk::Window* ownerWindow(IWindow* parent) {
    if (!parent) {
        return nullptr;
    }
    Window* parentWindow = dynamic_cast<Window*>(parent);
    if (!parentWindow || !parentWindow->getGtkWindow()) {
        throw std::runtime_error("Parent window is not a Window");
    }
    return parentWindow->getGtkWindow();
}

} // namespace

std::vector<std::string> showNativeFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) {
    Gtk::Window* owner = ownerWindow(parent);
    const Gtk::FileChooserAction action = dialogAction(descriptor);

    std::unique_ptr<Gtk::FileChooserDialog> dialog;
    if (owner) {
        dialog = std::make_unique<Gtk::FileChooserDialog>(*owner, dialogTitle(descriptor), action);
    } else {
        dialog = std::make_unique<Gtk::FileChooserDialog>(dialogTitle(descriptor), action);
    }

    dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog->add_button(acceptLabel(descriptor), Gtk::RESPONSE_ACCEPT);
    dialog->set_default_response(Gtk::RESPONSE_ACCEPT);

    if (descriptor.Mode == FileDialogMode::OpenFiles) {
        dialog->set_select_multiple(true);
    }
    if (descriptor.Mode == FileDialogMode::SaveFile) {
        dialog->set_do_overwrite_confirmation(true);
    }

    applyCommonSettings(*dialog, descriptor);
    const std::vector<AddedFilter> filters = addFilters(*dialog, descriptor);

    std::vector<std::string> paths;
    if (dialog->run() != Gtk::RESPONSE_ACCEPT) {
        dialog->hide();
        return paths;
    }

    if (descriptor.Mode == FileDialogMode::OpenFiles) {
        for (const std::string& filename : dialog->get_filenames()) {
            if (!filename.empty()) {
                paths.push_back(pathToUtf8(filename));
            }
        }
    } else {
        const std::string filename = dialog->get_filename();
        if (!filename.empty()) {
            std::string path = pathToUtf8(filename);
            if (descriptor.Mode == FileDialogMode::SaveFile) {
                path = withDefaultExtension(path, extensionForFilter(filters, dialog->get_filter()));
            }
            paths.push_back(std::move(path));
        }
    }

    dialog->hide();
    return paths;
}

} // namespace Luwow::Gui
