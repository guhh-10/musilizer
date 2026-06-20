#pragma once

#include <optional>

#include "config.hpp"
#include "model/track.hpp"

namespace MetadataReader {
    // Parse ID3v2 tags from an MP3 file and return a Track.
    // Returns std::nullopt if the file is invalid or has no ID3v2 tag.
    std::optional<Track> readTrack(const fs::path& music_path);
}