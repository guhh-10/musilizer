#include <iostream>
#include <algorithm>

#include "model/music_directory.hpp"
#include "model/library.hpp"
#include "service/metadata_reader.hpp"
#include "config.hpp"

void MusicDirectory::loadMetadata(fs::path music_path, Library& lib) {
    auto track = MetadataReader::readTrack(music_path);
    if (track.has_value())
        lib.addTrack(std::move(*track));
}

void MusicDirectory::initialize(Library& lib) {
    if (!fs::exists(config::MUSIC_DIR)) {
        std::cerr << "[MusicDirectory] music directory not found: "
                  << config::MUSIC_DIR << " — library will be empty\n";
        return;
    }

    if (!fs::is_directory(config::MUSIC_DIR)) {
        std::cerr << "[MusicDirectory] path is not a directory: "
                  << config::MUSIC_DIR << " — library will be empty\n";
        return;
    }

    try {
        for (const auto& entry : fs::recursive_directory_iterator(
                config::MUSIC_DIR,
                fs::directory_options::skip_permission_denied))
        {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mp3")
                loadMetadata(entry.path(), lib);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[MusicDirectory] error reading directory: "
                  << e.what() << " — partial library loaded\n";
    }
}