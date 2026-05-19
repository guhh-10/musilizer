#pragma once
#include <filesystem>

#include "model/library.hpp"

namespace fs = std::filesystem;

class musicDirectory{
    public:
        static inline const fs::path MUSIC_FOLDER = fs::current_path() / "music";

        void loadMetadata(fs::path musicpath, library& lib);
        void initialize(library& lib);
};