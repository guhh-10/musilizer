#include <fstream>
#include <nlohmann/json.hpp>
#include <exception>
#include <iostream>

#include "repository/persistence.hpp"
#include "model/library.hpp"
#include "config.hpp"

using json = nlohmann::json;

void Persistence::init() {
    try {
        fs::create_directories(config::DATA_DIR);

        auto writeDefault = [](const fs::path& path, const nlohmann::json& j) {
            std::ofstream f(path);
            if (!f.is_open()) {
                std::cerr << "[Persistence::init] failed to create: " << path << "\n";
                return;
            }
            f << j;
            if (!f)
                std::cerr << "[Persistence::init] write error: " << path << "\n";
        };

        // fs::exists() can throw on permission errors or malformed paths.
        // Wrap each check individually so one failure doesn't block the others.

        try {
            if (!fs::exists(config::SETTING)) {
                nlohmann::json j;
                j["volume"]  = 1.0f;
                j["shuffle"] = false;
                j["repeat"]  = false;
                writeDefault(config::SETTING, j);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[Persistence::init] could not check/create settings: "
                      << e.what() << "\n";
        }

        try {
            if (!fs::exists(config::PLAYLIST)) {
                nlohmann::json j;
                j["playlists"] = nlohmann::json::array();
                writeDefault(config::PLAYLIST, j);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[Persistence::init] could not check/create playlists: "
                      << e.what() << "\n";
        }

        try {
            if (!fs::exists(config::HISTORY)) {
                nlohmann::json j;
                j["history"] = nlohmann::json::array();
                writeDefault(config::HISTORY, j);
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[Persistence::init] could not check/create history: "
                      << e.what() << "\n";
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "[Persistence::init] failed to create data directory: "
                  << e.what() << "\n";
    }
}

void Persistence::saveSettings(float volume, bool shuffle, bool repeat) {
    nlohmann::json j;
    j["volume"]  = volume;
    j["shuffle"] = shuffle;
    j["repeat"]  = repeat;
    std::ofstream f(config::SETTING);
    if (!f.is_open()) {
        std::cerr << "[Persistence::saveSettings] failed: cannot open settings file for write\n";
        return;
    }
    f << j;
    if (!f)
        std::cerr << "[Persistence::saveSettings] failed: write error\n";
}

void Persistence::loadSettings(float& volume, bool& shuffle, bool& repeat) {
    try {
        std::ifstream f(config::SETTING);
        if (!f.is_open()) throw std::runtime_error("cannot open settings file");
        nlohmann::json j = nlohmann::json::parse(f);
        volume  = j.at("volume");
        shuffle = j.at("shuffle");
        repeat  = j.at("repeat");
    } catch (const std::exception& e) {
        std::cerr << "[Persistence::loadSettings] failed: " << e.what() << " — using defaults\n";
        volume  = 1.0f;
        shuffle = false;
        repeat  = false;
    }
}

void Persistence::savePlaylists(const std::vector<Playlist>& playlists) {
    nlohmann::json j;
    j["playlists"] = nlohmann::json::array();
    for (const Playlist& p : playlists) {
        nlohmann::json entry;
        entry["name"] = p.getName();
        std::vector<std::string> pathStrings;
        for (const fs::path& path : p.getPlaylistTracks())
            pathStrings.push_back(path.string());
        entry["tracks"] = pathStrings;
        j["playlists"].push_back(entry);
    }
    std::ofstream f(config::PLAYLIST);
    if (!f.is_open()) {
        std::cerr << "[Persistence::savePlaylists] failed: cannot open playlists file for write\n";
        return;
    }
    f << j;
    if (!f)
        std::cerr << "[Persistence::savePlaylists] failed: write error\n";
}

std::vector<Playlist> Persistence::loadPlaylists(const Library& lib) {
    try {
        std::ifstream f(config::PLAYLIST);
        if (!f.is_open()) throw std::runtime_error("cannot open playlists file");
        nlohmann::json j = nlohmann::json::parse(f);
        std::vector<Playlist> result;
        for (const auto& entry : j.at("playlists")) {
            Playlist p(entry.at("name"));
            for (const auto& path : entry.at("tracks")) {
                const Track* t = lib.findByPath(fs::path(path));
                if (t) p.addTrack(*t);
            }
            result.push_back(p);
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << "[Persistence::loadPlaylists] failed: " << e.what() << " — returning empty\n";
        return {};
    }
}

void Persistence::saveHistory(const PlayHistory& history) {
    nlohmann::json j;
    std::vector<std::string> pathStrings;
    for (const fs::path& p : history.getHistory())
        pathStrings.push_back(p.string());
    j["history"] = pathStrings;
    std::ofstream f(config::HISTORY);
    if (!f.is_open()) {
        std::cerr << "[Persistence::saveHistory] failed: cannot open history file for write\n";
        return;
    }
    f << j;
    if (!f)
        std::cerr << "[Persistence::saveHistory] failed: write error\n";
}

void Persistence::loadHistory(PlayHistory& history, const Library& lib) {
    try {
        std::ifstream f(config::HISTORY);
        if (!f.is_open()) throw std::runtime_error("cannot open history file");
        nlohmann::json j = nlohmann::json::parse(f);
        for (const auto& path : j.at("history")) {
            const Track* t = lib.findByPath(fs::path(path));
            if (t) history.push(*t);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Persistence::loadHistory] failed: " << e.what()
                  << " — starting with empty history\n";
    }
}

void Persistence::saveLearner(const GenreGraphLearner& learner) {
    nlohmann::json j;
    j["observed"] = nlohmann::json::object();

    for (const auto& [from, tos] : learner.observedCounts()) {
        j["observed"][from] = nlohmann::json::object();
        for (const auto& [to, count] : tos)
            j["observed"][from][to] = count;
    }

    std::ofstream f(config::LEARNER);
    if (!f.is_open()) {
        std::cerr << "[Persistence::saveLearner] failed: cannot open learner file for write\n";
        return;
    }
    f << j;
    if (!f)
        std::cerr << "[Persistence::saveLearner] failed: write error\n";
}

void Persistence::loadLearner(GenreGraphLearner& learner) {
    try {
        std::ifstream f(config::LEARNER);
        if (!f.is_open()) return; // first run — nothing to load, prior is enough

        nlohmann::json j = nlohmann::json::parse(f);

        GenreGraphLearner::CountTable counts;
        for (const auto& [from, tos] : j.at("observed").items())
            for (const auto& [to, count] : tos.items())
                counts[from][to] = count.get<float>();

        learner.setObservedCounts(std::move(counts));
    } catch (const std::exception& e) {
        std::cerr << "[Persistence::loadLearner] failed: " << e.what()
                  << " — starting with prior only\n";
    }
}