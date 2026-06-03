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

// file-local helpers

// Split `s` on `delim`, trim ASCII whitespace from each token, drop empties.
static std::vector<std::string> splitTrim(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim)) {
        // trim leading whitespace
        auto first = token.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        // trim trailing whitespace
        auto last = token.find_last_not_of(" \t\r\n");
        out.push_back(token.substr(first, last - first + 1));
    }
    return out;
}

// MusicDirectory

void MusicDirectory::loadMetadata(fs::path music_path, Library& lib) {
    TagLib::MPEG::File file(music_path.string().c_str());

    if (!file.isValid() || !file.ID3v2Tag())
        return;

    TagLib::ID3v2::Tag* tag = file.ID3v2Tag();

    // title
    std::string title;
    auto tit2 = tag->frameListMap()["TIT2"];
    if (!tit2.isEmpty())
        title = tit2.front()->toString().toCString(true);

    // artists
    // Preferred: TXXX[ARTISTS], null-separated; fallback: TPE1.
    std::vector<std::string> artists;
    auto txxx = tag->frameListMap()["TXXX"];
    for (const auto& frame : txxx) {
        std::string raw = frame->toString().toCString(true);
        if (raw.rfind("[ARTISTS]", 0) == 0) {
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
    if (artists.empty()) {
        auto tpe1 = tag->frameListMap()["TPE1"];
        if (!tpe1.isEmpty())
            artists.push_back(tpe1.front()->toString().toCString(true));
    }

    // genres
    // TCON frame holds a free-text string.  Multiple genres are separated by
    // semicolons, e.g. "Rock;Alternative;Indie".  We trim each token and drop
    // empty entries so stray delimiters never produce blank genre strings.
    std::vector<std::string> genres;
    auto tcon = tag->frameListMap()["TCON"];
    if (!tcon.isEmpty()) {
        std::string raw = tcon.front()->toString().toCString(true);
        genres = splitTrim(raw, ';');
    }

    // duration
    int duration = 0;
    if (file.audioProperties())
        duration = file.audioProperties()->lengthInSeconds();

    lib.addTrack(Track(std::move(artists),
                       std::move(music_path),
                       std::move(title),
                       duration,
                       std::move(genres)));
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