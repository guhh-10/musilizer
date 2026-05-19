#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <sstream>
#include <string>

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

    // artists — split TPE2 on "; " into vector
    std::vector<std::string> artists;
    auto tpe2 = tag->frameListMap()["TPE2"];
    if (!tpe2.isEmpty()) {
        std::string raw = tpe2.front()->toString().toCString(true);
        std::stringstream ss(raw);
        std::string token;
        while (std::getline(ss, token, ';')) {
            // trim leading space
            if (!token.empty() && token.front() == ' ')
                token.erase(0, 1);
            artists.push_back(token);
        }
    } else {
        // fallback to TPE1 as single artist
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