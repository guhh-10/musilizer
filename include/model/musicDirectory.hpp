#pragma once
#include <filesystem>

#include "model/library.hpp"

namespace fs = std::filesystem;

class musicDirectory{
    public:
        void loadMetadata(fs::path musicpath, library& lib);
        void initialize(library& lib);
};