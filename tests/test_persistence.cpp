#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

#include "test_helpers.hpp"
#include "repository/persistence.hpp"
#include "model/library.hpp"
#include "model/play_history.hpp"
#include "model/playlist.hpp"

using namespace test_helpers;

TEST_CASE("Persistence init creates default files", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_default");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    CHECK(fs::exists(config::SETTING));
    CHECK(fs::exists(config::PLAYLIST));
    CHECK(fs::exists(config::HISTORY));
}

TEST_CASE("Persistence settings round-trip", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_settings");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    Persistence::saveSettings(0.25f, true, true);

    float volume = 0.0f;
    bool shuffle = false;
    bool repeat = false;
    Persistence::loadSettings(volume, shuffle, repeat);

    CHECK(volume == Catch::Approx(0.25f));
    CHECK(shuffle);
    CHECK(repeat);
}

TEST_CASE("Persistence handles missing settings file", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_missing");
    ScopedConfigRoot scoped(root);

    Persistence::init();
    fs::remove(config::SETTING);

    float volume = 0.0f;
    bool shuffle = true;
    bool repeat = true;
    Persistence::loadSettings(volume, shuffle, repeat);

    CHECK(volume == Catch::Approx(1.0f));
    CHECK_FALSE(shuffle);
    CHECK_FALSE(repeat);
}

TEST_CASE("Persistence init does not overwrite existing files", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_reinit");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::DATA_DIR);
    std::string sentinelSettings = "settings-sentinel";
    std::string sentinelPlaylist = "playlist-sentinel";
    std::string sentinelHistory = "history-sentinel";

    std::ofstream(config::SETTING) << sentinelSettings;
    std::ofstream(config::PLAYLIST) << sentinelPlaylist;
    std::ofstream(config::HISTORY) << sentinelHistory;

    Persistence::init();

    CHECK(readFileText(config::SETTING) == sentinelSettings);
    CHECK(readFileText(config::PLAYLIST) == sentinelPlaylist);
    CHECK(readFileText(config::HISTORY) == sentinelHistory);
}

TEST_CASE("Persistence handles malformed settings JSON", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_malformed_settings");
    ScopedConfigRoot scoped(root);

    Persistence::init();
    std::ofstream(config::SETTING) << "{not json";

    float volume = 0.0f;
    bool shuffle = true;
    bool repeat = true;
    Persistence::loadSettings(volume, shuffle, repeat);
    CHECK(volume == Catch::Approx(1.0f));
    CHECK_FALSE(shuffle);
    CHECK_FALSE(repeat);
}

TEST_CASE("Persistence handles malformed playlists JSON", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_malformed_playlists");
    ScopedConfigRoot scoped(root);

    Persistence::init();
    std::ofstream(config::PLAYLIST) << "[not json";

    Library lib;
    auto playlists = Persistence::loadPlaylists(lib);
    CHECK(playlists.empty());
}

TEST_CASE("Persistence handles malformed history JSON", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_malformed_history");
    ScopedConfigRoot scoped(root);

    Persistence::init();
    std::ofstream(config::HISTORY) << "not json";

    Library lib;
    PlayHistory history;
    Persistence::loadHistory(history, lib);
    CHECK(history.getHistory().empty());
    CHECK_FALSE(history.current().has_value());
}

TEST_CASE("Persistence handles empty playlists", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_empty_playlist");
    ScopedConfigRoot scoped(root);

    Persistence::init();
    Persistence::savePlaylists({});

    Library lib;
    auto loaded = Persistence::loadPlaylists(lib);
    CHECK(loaded.empty());

    nlohmann::json j = nlohmann::json::parse(readFileText(config::PLAYLIST));
    REQUIRE(j.contains("playlists"));
    CHECK(j.at("playlists").is_array());
    CHECK(j.at("playlists").empty());
}

TEST_CASE("Persistence playlist round-trip", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_playlist");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    Library lib;
    Track t1 = makeTrack("a.mp3", 10);
    Track t2 = makeTrack("b.mp3", 20);
    lib.addTrack(t1);
    lib.addTrack(t2);

    Playlist p1("Favorites");
    p1.addTrack(t1);
    p1.addTrack(t2);

    Playlist p2("Missing");
    p2.addTrack(makeTrack("missing.mp3", 30));

    Persistence::savePlaylists({p1, p2});

    auto loaded = Persistence::loadPlaylists(lib);
    REQUIRE(loaded.size() == 2);
    // loadPlaylists preserves JSON array order
    CHECK(loaded[0].getName() == "Favorites");
    CHECK(loaded[0].size() == 2);
    CHECK(loaded[1].getName() == "Missing");
    CHECK(loaded[1].size() == 0);
}

TEST_CASE("Persistence history round-trip", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_history");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    Library lib;
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    lib.addTrack(t1);
    lib.addTrack(t2);

    PlayHistory history;
    history.push(t1);
    history.push(t2);

    Persistence::saveHistory(history);

    PlayHistory loaded;
    Persistence::loadHistory(loaded, lib);

    REQUIRE(loaded.getHistory().size() == 2);
    CHECK(loaded.current().has_value());
    CHECK(loaded.current().value() == fs::path("b.mp3"));
}

TEST_CASE("Persistence history skips missing library tracks", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_history_missing");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    Library lib;
    Track t1 = makeTrack("present.mp3");
    lib.addTrack(t1);

    PlayHistory history;
    history.push(t1);
    history.push(makeTrack("missing.mp3"));

    Persistence::saveHistory(history);

    PlayHistory loaded;
    Persistence::loadHistory(loaded, lib);

    CHECK(loaded.getHistory().size() == 1);
    REQUIRE(loaded.current().has_value());
    CHECK(loaded.current().value() == fs::path("present.mp3"));
}
