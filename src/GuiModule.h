#pragma once

#include "IGuiModule.h"
#include "ILuauHost.h"
#include "MainThreadCall.h"

#include <atomic>
#include <memory>
#include <thread>

namespace Luwow::Gui {

// The gui module, the same on every platform but for its main loop, which each platform's Platform file implements
class GuiModule : public IGuiModule {
public:
    const char* getModuleName() const override;
    const char* getModuleAlias() const override;
    const LuauExport* getExports() const override;
    Luwow::Engine::ILuauModule* initialize(Luwow::Engine::ILuauHost* host) override;
    Luwow::Engine::RunMode getRunMode() const override;
    Luwow::Engine::ILuauHost* getHost() const override { return host; }

    bool isMainThread() const override { return std::this_thread::get_id() == mainThread; }
    bool runOnMainThread(const std::function<void()>& work) override;
    // Runs the main loop until the shown windows are closed, returning at once when none is shown
    void run() override;

    IWindow* createWindow(const WindowDescriptor& descriptor) override;
    IButton* createButton(const ButtonDescriptor& descriptor, IWindow* window) override;
    IMenuBar* createMenuBar(IWindow* window) override;
    IMenu* createMenu(const MenuDescriptor& descriptor) override;
    IMenuItem* createMenuItem(const MenuItemDescriptor& descriptor, IWindow* window) override;
    std::vector<std::string> showFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent) override;

private:
    // The platform's part
    // Readies the platform for windows, on the main thread
    void startPlatform();
    // Queues the call on the main loop, false when it can't
    bool post(const std::shared_ptr<MainThreadCall>& call);
    // Runs the main loop until quitMainLoop
    void runLoop();

    Luwow::Engine::ILuauHost* host = nullptr;
    std::thread::id mainThread;
    std::atomic<bool> loopEnded = false;
};

} // namespace Luwow::Gui
