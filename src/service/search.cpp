#include <algorithm>

#include "service/search.hpp"
#include "utils/string_utils.hpp"

// We use utils::toLower to avoid UB with non-ASCII characters
static auto lower = [](std::string s) {
    return utils::toLower(std::move(s));
};

std::vector<std::string> Search::tokenize(const std::string& s) {
    std::vector<std::string> tokens;
    std::string cur;
    for (unsigned char c : s) {
        if (std::isalnum(c)) {
            cur.push_back(static_cast<char>(c));
        } else{
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

float Search::scoreTrack(const Track& t, const std::string& lowerText) const {
    if (lowerText.empty()) return 1.0f;
 
    const std::string title = lower(t.getTitle());
 
    if (title.rfind(lowerText, 0) == 0) return 3.0f;
    if (title.find(lowerText) != std::string::npos) return 2.0f;
 
    for (const auto& artist : t.getArtists())
        if (lower(artist).find(lowerText) != std::string::npos)
            return 1.0f;
 
    return 0.0f;
}

Search::Search(const Library& lib) {
    rebuild(lib);
}

void Search::rebuild(const Library& lib) {
    title_index.clear();
    artist_index.clear();

    for (const auto& [path, track] : lib.getTracks()) {
        const fs::path& p = track.getMusicPath();
 
        for (const std::string& word : tokenize(track.getTitle()))
            title_index[lower(word)].insert(p);
 
        for (const std::string& artist : track.getArtists())
            artist_index[lower(artist)].insert(p);
    }
}

std::vector<SearchResult> Search::query(const Library& lib, const SearchQuery& q) const {
    const std::string lowerText = lower(q.text);
 
    // 1. candidate set from the index
 
    std::set<fs::path> candidates;
 
    if (!lowerText.empty()) {
        // title token hits — prefix scan from lower_bound
        for (const std::string& token : tokenize(lowerText)) {
            auto it = title_index.lower_bound(token);           
            while (it != title_index.end() &&
                   it->first.substr(0, token.size()) == token) {
                candidates.insert(it->second.begin(), it->second.end());
                ++it;
            }
        }
 
        // artist hits — substring scan around lower_bound
        auto it = artist_index.lower_bound(lowerText);          
        while (it != artist_index.end()) {
            if (it->first.find(lowerText) != std::string::npos)
                candidates.insert(it->second.begin(), it->second.end());
            if (it->first > lowerText + "~") break;
            ++it;
        }
        if (it != artist_index.begin()) {
            auto rit = std::make_reverse_iterator(it);
            while (rit != artist_index.rend()) {
                if (rit->first.find(lowerText) != std::string::npos)
                    candidates.insert(rit->second.begin(), rit->second.end());
                else if (rit->first < lowerText)
                    break;
                ++rit;
            }
        }
    } else {
        for (const auto& [path, track] : lib.getTracks())
            candidates.insert(track.getMusicPath());
    }
 
    // 2. filter and score
 
    std::vector<SearchResult> results;
 
    for (const fs::path& path : candidates) {
        const Track* track = lib.findByPath(path);
        if (!track) continue;
 
        float score = scoreTrack(*track, lowerText);
        if (!lowerText.empty() && score == 0.0f) continue;
 
        if (!q.artistFilter.empty()) {
            bool match = false;
            for (const auto& a : track->getArtists())
                if (lower(a) == lower(q.artistFilter)) { match = true; break; }
            if (!match) continue;
        }
 
        int dur = track->getDuration();
        if (q.minDuration > 0 && dur < q.minDuration) continue;
        if (q.maxDuration > 0 && dur > q.maxDuration) continue;
 
        results.push_back({track, score});
    }
 
    // 3. sort
 
    std::sort(results.begin(), results.end(),
        [&](const SearchResult& a, const SearchResult& b) {
            if (!lowerText.empty() && a.score != b.score)
                return a.score > b.score;
 
            auto cmpStr = [&](const std::string& sa, const std::string& sb) {
                return q.sortOrder == SortOrder::ASC ? sa < sb : sa > sb;
            };
            auto cmpInt = [&](int ia, int ib) {
                return q.sortOrder == SortOrder::ASC ? ia < ib : ia > ib;
            };
 
            switch (q.sortBy) {
                case SortField::TITLE:
                    return cmpStr(lower(a.track->getTitle()),
                                  lower(b.track->getTitle()));
                case SortField::ARTIST: {
                    const std::string aa = a.track->getArtists().empty() ? ""
                                           : lower(a.track->getArtists().front());
                    const std::string ba = b.track->getArtists().empty() ? ""
                                           : lower(b.track->getArtists().front());
                    return cmpStr(aa, ba);
                }
                case SortField::DURATION:
                    return cmpInt(a.track->getDuration(), b.track->getDuration());
            }
            return false;
        });
 
    return results;
}

std::vector<std::string> Search::allArtists() const {
    std::vector<std::string> out;
    out.reserve(artist_index.size());
    for (const auto& [artist, _] : artist_index)
        out.push_back(artist);
    return out;
}
