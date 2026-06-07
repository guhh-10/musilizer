#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace config {
    inline fs::path ROOT;
    inline fs::path MUSIC_DIR;
    inline fs::path DATA_DIR;
    inline fs::path PLAYLIST;
    inline fs::path SETTING;
    inline fs::path HISTORY;
    inline fs::path LEARNER;

    inline void init(const fs::path& binaryDir) {
        ROOT      = binaryDir;
        MUSIC_DIR = ROOT / "music";
        DATA_DIR  = ROOT / "data";
        PLAYLIST  = DATA_DIR / "playlist.json";
        SETTING   = DATA_DIR / "setting.json";
        HISTORY   = DATA_DIR / "history.json";
        LEARNER   = DATA_DIR / "learner.json";
    }

    inline constexpr int MAX_HISTORY = 50;
}