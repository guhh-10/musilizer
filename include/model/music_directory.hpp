#pragma once

#include "config.hpp"
#include "model/library.hpp"

class MusicDirectory {
    public:
        void loadMetadata(fs::path music_path, Library& lib);
        void initialize(Library& lib);
};