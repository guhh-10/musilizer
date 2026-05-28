#pragma once
#include <filesystem>

namespace fs = std::filesystem;

namespace config{
    inline const fs::path ROOT = fs::current_path();
    inline const fs::path MUSIC_DIR = ROOT / "music";
    inline const fs::path DATA_DIR = ROOT / "data";
    inline const fs::path PLAYLIST = DATA_DIR / "playlist.json";
    inline const fs::path SETTING = DATA_DIR / "setting.json";
    inline const fs::path HISTORY = DATA_DIR / "history.json";
}
