#pragma once

#include <cstdio>
#include <string>

namespace utils {

inline std::string formatDuration(int secs) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", secs / 60, secs % 60);
    return buf;
}

} // namespace utils
