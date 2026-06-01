#pragma once

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <audioproperties.h>

#include "config.hpp"
#include "model/track.hpp"

namespace test_helpers {
struct ScopedConfigRoot {
    fs::path old_root;
    fs::path old_music;
    fs::path old_data;
    fs::path old_playlist;
    fs::path old_setting;
    fs::path old_history;

    explicit ScopedConfigRoot(const fs::path& root) {
        old_root = config::ROOT;
        old_music = config::MUSIC_DIR;
        old_data = config::DATA_DIR;
        old_playlist = config::PLAYLIST;
        old_setting = config::SETTING;
        old_history = config::HISTORY;
        config::init(root);
    }

    ~ScopedConfigRoot() {
        config::ROOT = old_root;
        config::MUSIC_DIR = old_music;
        config::DATA_DIR = old_data;
        config::PLAYLIST = old_playlist;
        config::SETTING = old_setting;
        config::HISTORY = old_history;
    }
};

inline Track makeTrack(const fs::path& path, int duration = 120) {
    return Track({"Artist"}, path, "Title", duration);
}

inline fs::path makeTempRoot(const std::string& name) {
    fs::path root = fs::temp_directory_path() / name;
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

inline fs::path findFixture(const fs::path& relative) {
    fs::path cwd = fs::current_path();
    fs::path direct = cwd / relative;
    if (fs::exists(direct))
        return direct;

    fs::path parent = cwd.parent_path() / relative;
    if (fs::exists(parent))
        return parent;

    return {};
}

inline std::string readFileText(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

inline void writeSampleMp3(const fs::path& path, std::size_t frameCount = 39) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());

    std::vector<unsigned char> frame(417, 0);
    frame[0] = 0xFF;
    frame[1] = 0xFB;
    frame[2] = 0x90;
    frame[3] = 0x64;

    for (std::size_t i = 0; i < frameCount; ++i)
        out.write(reinterpret_cast<const char*>(frame.data()), static_cast<std::streamsize>(frame.size()));
}

inline void tagSampleMp3(const fs::path& path, const std::string& title, const std::string& artist) {
    TagLib::MPEG::File file(path.string().c_str());
    REQUIRE(file.isValid());

    TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);
    REQUIRE(tag != nullptr);

    tag->setTitle(title);
    tag->setArtist(artist);
    REQUIRE(file.save());
}
} // namespace test_helpers
