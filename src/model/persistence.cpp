#include <fstream>
#include <nlohmann/json.hpp>

#include "model/persistence.hpp"
#include "model/library.hpp"
#include "config.hpp"

using json = nlohmann::json;

void persistence::init(){
    fs::create_directories(config::DATA_DIR);

    if(!fs::exists(config::SETTING)){
        nlohmann::json j;
        j["volume"] = 1.0f;
        j["shuffle"] = false;
        j["repeat"] = false;
        std::ofstream(config::SETTING) << j;
    }
    
    if(!fs::exists(config::PLAYLIST)){
        nlohmann::json j;
        j["playlists"] = nlohmann::json::array();
        std::ofstream(config::PLAYLIST) << j;
    }
    
    if(!fs::exists(config::HISTORY)){
    nlohmann::json j;
    j["history"] = nlohmann::json::array();
    std::ofstream(config::HISTORY) << j;
}

}

void persistence::saveSettings(float volume, bool shuffle, bool repeat){
    nlohmann::json j;
    j["volume"] = volume;
    j["shuffle"] = shuffle;
    j["repeat"] = repeat;
    std::ofstream(config::SETTING) << j;
}

void persistence::loadSettings(float& volume, bool& shuffle, bool& repeat){
    std::ifstream f(config::SETTING);
    nlohmann::json j = nlohmann::json::parse(f);
    volume  = j["volume"];
    shuffle = j["shuffle"];
    repeat  = j["repeat"];
}

void persistence::savePlaylists(const std::vector<playlist>& playlists){
    nlohmann::json j;
    j["playlists"] = nlohmann::json::array();
    for(const playlist& p : playlists){
        nlohmann::json entry;
        entry["name"] = p.getName();
        entry["tracks"] = p.getPlaylistTracks();
        j["playlists"].push_back(entry);
    }
    std::ofstream(config::PLAYLIST) << j;
}

std::vector<playlist> persistence::loadPlaylists(const library& lib){
    std::ifstream f(config::PLAYLIST);
    nlohmann::json j = nlohmann::json::parse(f);
    std::vector<playlist> result;
    for(const auto& entry : j["playlists"]){
        playlist p(entry["name"]);
        for(const std::string& path : entry["tracks"]){
            const track* t = lib.findByPath(path);
            if(t) p.addTrack(*t);
        }
        result.push_back(p);
    }
    return result;
}

void persistence::saveHistory(const playHistory& history){
    nlohmann::json j;
    j["history"] = history.getHistory();
    std::ofstream(config::HISTORY) << j;
}

void persistence::loadHistory(playHistory& history, const library& lib){
    std::ifstream f(config::HISTORY);
    nlohmann::json j = nlohmann::json::parse(f);
    for(const std::string& path : j["history"]){
        const track* t = lib.findByPath(path);
        if(t) history.push(*t);
    }
}
