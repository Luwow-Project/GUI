#pragma once
#include "IGuiModule.h"
#include "ILuauHost.h"

#include <atomic>
#include <thread>

namespace Luwow::Gui {

using ILuauHost = Luwow::Engine::ILuauHost;
using RunMode = Luwow::Engine::RunMode;

class GuiModule : public IGuiModule {
public:
    GuiModule();
    ~GuiModule() = default;

    const char* getModuleName() const override;
    const char* getModuleAlias() const override;
    const LuauExport* getExports() const override;
    ILuauModule* initialize(ILuauHost* host) override;
    IWindow* createWindow(const WindowDescriptor& descriptor) override;
    IButton* createButton(const ButtonDescriptor& descriptor, IWindow* window) override;
    IMenuBar* createMenuBar(IWindow* window) override;
    IMenu* createMenu(const MenuDescriptor& descriptor) override;
    IMenuItem* createMenuItem(const MenuItemDescriptor& descriptor, IWindow* window) override;
    std::vector<std::string> showFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) override;

    RunMode getRunMode() const override;
    ILuauHost* getHost() const override { return host; }

    bool isMainThread() const override { return std::this_thread::get_id() == mainThread; }
    bool runOnMainThread(const std::function<void()>& work) override;
    // Runs the window message loop on the main thread until the windows are closed
    void run() override;
private:
    void setHost(ILuauHost* host);
    ILuauHost* host;
    std::thread::id mainThread;
    std::atomic<bool> loopEnded = false;
};

} // namespace Luwow::Gui
