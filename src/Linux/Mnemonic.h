#pragma once

#include <string>

namespace Luwow::Gui {

// Windows descriptors mark a mnemonic with '&' ("&File", and "&&" for a literal '&').
// GTK uses '_' in that position, and "__" for a literal '_'.
inline std::string windowsMnemonicToGtk(const std::string& title) {
    std::string result;
    result.reserve(title.size());
    for (size_t i = 0; i < title.size(); ++i) {
        if (title[i] == '&' && i + 1 < title.size()) {
            if (title[i + 1] == '&') {
                result.push_back('&');
                ++i;
            } else {
                result.push_back('_');
            }
            continue;
        }
        if (title[i] == '_') {
            result.push_back('_');
        }
        result.push_back(title[i]);
    }
    return result;
}

} // namespace Luwow::Gui
