#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace utils {

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

} // namespace utils
