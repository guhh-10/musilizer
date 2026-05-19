#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <sstream>
#include <string>
#include <iostream>

#include "model/musicDirectory.hpp"
#include "model/track.hpp"
#include "model/library.hpp"

void musicDirectory::loadMetadata(fs::path musicpath, library& lib){
    TagLib::MPEG::File file(musicpath.string().c_str());

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

    lib.addTrack(track(std::move(artists), std::move(musicpath), std::move(title), duration));
}

void musicDirectory::initialize(library& lib){
    for( const auto& entry : fs::directory_iterator(MUSIC_FOLDER)){
        if(entry.path().extension() == ".mp3"){
            loadMetadata(entry.path(), lib);
        }
    }
}