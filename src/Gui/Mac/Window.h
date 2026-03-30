#pragma once

#include "IGuiModule.h"
#include <unordered_map>

namespace Luwow::Gui {

class Window : public IWindow {
public:
    Window(const WindowDescriptor& descriptor);
    ~Window() override;
    void show() override;
    uint16_t registerCommandControl(ICommandControl* commandControl) override;
    ICommandControl* getCommandControl(uint16_t id) const override;

    WindowDescriptor getDescriptor() const { return descriptor; }
    void* getNativeContentView() const;

private:
    uint16_t nextCommandId = 1;
    std::unordered_map<uint16_t, ICommandControl*> commandControls;
    WindowDescriptor descriptor;
    void* nativeWindow = nullptr;
    void* nativeDelegate = nullptr;
};

void getWindowTable(lua_State* L, Window* window);

} // namespace Luwow::Gui
