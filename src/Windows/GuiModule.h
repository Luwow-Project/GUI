#pragma once
#include "IGuiModule.h"
#include "ILuauHost.h"

namespace Luwow::Gui {

using ILuauHost = Luwow::Engine::ILuauHost;

class GuiModule : public IGuiModule {
public:
    GuiModule();
    ~GuiModule() = default;

    const char* getModuleName() const override;
    const char* getModuleAlias() const override;
    const LuauExport* getExports() const override;
    ILuauModule* initialize(ILuauHost* host) override;
    IWindow* createWindow(const WindowDescriptor& descriptor) override;
    IButton* createButton(const ButtonDescriptor& descriptor, IWindow* parent) override;
    IMenuBar* createMenuBar(const MenuBarDescriptor& descriptor, IWindow* parent) override;
    static void MessagePump();

private:
    void setHost(ILuauHost* host);
    ILuauHost* host;
};

} // namespace Luwow::Gui
