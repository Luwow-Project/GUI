#pragma once

#include "Descriptors.h"
#include "IGuiModule.h"
#include <string>
#include <vector>

namespace Luwow::Gui {

std::vector<std::string> showNativeFileDialog(const FileDialogDescriptor& descriptor, IWindow* parent);

} // namespace Luwow::Gui
