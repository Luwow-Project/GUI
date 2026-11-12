#pragma once

#include <string>

namespace Luwow::Gui {

// Strip Windows-style mnemonics ("&File" → "File", "&&" → "&").
inline std::string stripMnemonic(const std::string& title) {
    std::string result;
    result.reserve(title.size());
    for (size_t i = 0; i < title.size(); ++i) {
        if (title[i] == '&' && i + 1 < title.size()) {
            result.push_back(title[++i]);
            continue;
        }
        if (title[i] != '&') {
            result.push_back(title[i]);
        }
    }
    return result;
}

} // namespace Luwow::Gui
