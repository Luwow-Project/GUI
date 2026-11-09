#include "FileDialog.h"
#include "Window.h"

#include <windows.h>
#include <shobjidl.h>
#include <string>
#include <vector>

namespace Luwow::Gui {
namespace {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return {};
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (size <= 0) {
        return {};
    }
    std::wstring wide(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), wide.data(), size);
    return wide;
}

std::string WideToUtf8(const wchar_t* wide) {
    if (!wide || !*wide) {
        return {};
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return {};
    }
    std::string utf8(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8.data(), size, nullptr, nullptr);
    return utf8;
}

std::wstring BuildFilterSpec(const FileDialogFilter& filter) {
    std::wstring spec;
    for (size_t i = 0; i < filter.Extensions.size(); ++i) {
        if (i > 0) {
            spec += L";";
        }
        spec += L"*.";
        spec += Utf8ToWide(filter.Extensions[i]);
    }
    return spec;
}

std::string GetPathFromShellItem(IShellItem* item) {
    PWSTR path = nullptr;
    HRESULT hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
    if (FAILED(hr) || !path) {
        return {};
    }
    std::string utf8 = WideToUtf8(path);
    CoTaskMemFree(path);
    return utf8;
}

HWND GetOwnerHwnd(IWindow* parent) {
    if (!parent) {
        return nullptr;
    }
    Window* parentWindow = dynamic_cast<Window*>(parent);
    return parentWindow ? parentWindow->getHWND() : nullptr;
}

struct FilterStorage {
    std::vector<std::wstring> names;
    std::vector<std::wstring> specs;
    std::vector<COMDLG_FILTERSPEC> filterSpecs;
};

void ApplyCommonDialogSettings(IFileDialog* dialog, const FileDialogDescriptor& descriptor, FilterStorage& filters) {
    if (!descriptor.Title.empty()) {
        dialog->SetTitle(Utf8ToWide(descriptor.Title).c_str());
    }

    if (!descriptor.InitialDirectory.empty()) {
        IShellItem* folder = nullptr;
        HRESULT hr = SHCreateItemFromParsingName(
            Utf8ToWide(descriptor.InitialDirectory).c_str(),
            nullptr,
            IID_PPV_ARGS(&folder));
        if (SUCCEEDED(hr) && folder) {
            dialog->SetFolder(folder);
            folder->Release();
        }
    }

    if (!descriptor.DefaultFileName.empty()) {
        dialog->SetFileName(Utf8ToWide(descriptor.DefaultFileName).c_str());
    }

    if (descriptor.Mode != FileDialogMode::OpenFolder && !descriptor.Filters.empty()) {
        filters.names.reserve(descriptor.Filters.size());
        filters.specs.reserve(descriptor.Filters.size());
        filters.filterSpecs.reserve(descriptor.Filters.size());

        for (const FileDialogFilter& filter : descriptor.Filters) {
            filters.names.push_back(Utf8ToWide(filter.Name));
            filters.specs.push_back(BuildFilterSpec(filter));
            filters.filterSpecs.push_back({ filters.names.back().c_str(), filters.specs.back().c_str() });
        }

        dialog->SetFileTypes((UINT)filters.filterSpecs.size(), filters.filterSpecs.data());
        dialog->SetFileTypeIndex(1);

        const std::string& firstExtension = descriptor.Filters.front().Extensions.front();
        if (firstExtension != "*") {
            dialog->SetDefaultExtension(Utf8ToWide(firstExtension).c_str());
        }
    }
}

std::vector<std::string> ShowOpenDialog(const FileDialogDescriptor& descriptor, HWND owner) {
    std::vector<std::string> paths;

    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr) || !dialog) {
        return paths;
    }

    FILEOPENDIALOGOPTIONS options = 0;
    dialog->GetOptions(&options);
    options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;

    switch (descriptor.Mode) {
        case FileDialogMode::OpenFile:
            options &= ~(FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS);
            options |= FOS_FILEMUSTEXIST;
            break;
        case FileDialogMode::OpenFiles:
            options &= ~FOS_PICKFOLDERS;
            options |= FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST;
            break;
        case FileDialogMode::OpenFolder:
            options &= ~FOS_ALLOWMULTISELECT;
            options |= FOS_PICKFOLDERS;
            break;
        case FileDialogMode::SaveFile:
            break;
    }
    dialog->SetOptions(options);

    FilterStorage filters;
    ApplyCommonDialogSettings(dialog, descriptor, filters);

    hr = dialog->Show(owner);
    if (SUCCEEDED(hr)) {
        if (descriptor.Mode == FileDialogMode::OpenFiles) {
            IShellItemArray* items = nullptr;
            hr = dialog->GetResults(&items);
            if (SUCCEEDED(hr) && items) {
                DWORD count = 0;
                items->GetCount(&count);
                for (DWORD i = 0; i < count; ++i) {
                    IShellItem* item = nullptr;
                    if (SUCCEEDED(items->GetItemAt(i, &item)) && item) {
                        std::string path = GetPathFromShellItem(item);
                        if (!path.empty()) {
                            paths.push_back(std::move(path));
                        }
                        item->Release();
                    }
                }
                items->Release();
            }
        } else {
            IShellItem* item = nullptr;
            hr = dialog->GetResult(&item);
            if (SUCCEEDED(hr) && item) {
                std::string path = GetPathFromShellItem(item);
                if (!path.empty()) {
                    paths.push_back(std::move(path));
                }
                item->Release();
            }
        }
    }

    dialog->Release();
    return paths;
}

std::vector<std::string> ShowSaveDialog(const FileDialogDescriptor& descriptor, HWND owner) {
    std::vector<std::string> paths;

    IFileSaveDialog* dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr) || !dialog) {
        return paths;
    }

    FILEOPENDIALOGOPTIONS options = 0;
    dialog->GetOptions(&options);
    options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT;
    options &= ~(FOS_ALLOWMULTISELECT | FOS_PICKFOLDERS | FOS_FILEMUSTEXIST);
    dialog->SetOptions(options);

    FilterStorage filters;
    ApplyCommonDialogSettings(dialog, descriptor, filters);

    hr = dialog->Show(owner);
    if (SUCCEEDED(hr)) {
        IShellItem* item = nullptr;
        hr = dialog->GetResult(&item);
        if (SUCCEEDED(hr) && item) {
            std::string path = GetPathFromShellItem(item);
            if (!path.empty()) {
                paths.push_back(std::move(path));
            }
            item->Release();
        }
    }

    dialog->Release();
    return paths;
}

} // namespace

std::vector<std::string> showNativeFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) {
    HRESULT comHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    HWND owner = GetOwnerHwnd(parent);

    std::vector<std::string> paths = descriptor.Mode == FileDialogMode::SaveFile
        ? ShowSaveDialog(descriptor, owner)
        : ShowOpenDialog(descriptor, owner);

    if (comHr == S_OK) {
        CoUninitialize();
    }
    return paths;
}

} // namespace Luwow::Gui
