#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "model/music_directory.hpp"
#include "model/track.hpp"
#include "model/library.hpp"
#include "config.hpp"

void MusicDirectory::loadMetadata(fs::path music_path, Library& lib){
    TagLib::MPEG::File file(music_path.string().c_str());

    if (!file.isValid() || !file.ID3v2Tag())
        return;

    TagLib::ID3v2::Tag* tag = file.ID3v2Tag();

    // title
    std::string title;
    auto tit2 = tag->frameListMap()["TIT2"];
    if (!tit2.isEmpty())
        title = tit2.front()->toString().toCString(true);

    // artists — read from TXXX [ARTISTS], null-separated; fallback to TPE1
    std::vector<std::string> artists;
    auto txxx = tag->frameListMap()["TXXX"];
    for (const auto& frame : txxx) {
        std::string raw = frame->toString().toCString(true);
        if (raw.rfind("[ARTISTS]", 0) == 0) {
            // value is after "[ARTISTS] "
            std::string value = raw.substr(10);
            std::stringstream ss(value);
            std::string token;
            while (std::getline(ss, token, '\0'))
                artists.push_back(token);
            if (artists.empty())
                artists.push_back(value);
            break;
        }
    }

    // fallback to TPE1
    if (artists.empty()) {
        auto tpe1 = tag->frameListMap()["TPE1"];
        if (!tpe1.isEmpty())
            artists.push_back(tpe1.front()->toString().toCString(true));
    }

    // duration
    int duration = 0;
    if (file.audioProperties())
        duration = file.audioProperties()->lengthInSeconds();

    lib.addTrack(Track(std::move(artists), std::move(music_path), std::move(title), duration));
}

void MusicDirectory::initialize(Library& lib){
    if(!fs::exists(config::MUSIC_DIR)){
        std::cerr << "[MusicDirectory] music directory not found: "
                  << config::MUSIC_DIR << " — library will be empty\n";
        return;
    }

    if(!fs::is_directory(config::MUSIC_DIR)){
        std::cerr << "[MusicDirectory] path is not a directory: "
                  << config::MUSIC_DIR << " — library will be empty\n";
        return;
    }

    try {
        for(const auto& entry : fs::recursive_directory_iterator(
                config::MUSIC_DIR,
                fs::directory_options::skip_permission_denied))
        {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if(ext == ".mp3")
                loadMetadata(entry.path(), lib);
        }
    } catch(const fs::filesystem_error& e){
        std::cerr << "[MusicDirectory] error reading directory: "
                  << e.what() << " — partial library loaded\n";
    }
}