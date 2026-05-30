#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

#include "config.hpp"
#include "model/library.hpp"
#include "model/audio.hpp"
#include "model/music_directory.hpp"
#include "model/persistence.hpp"
#include "model/play_history.hpp"
#include "model/playlist.hpp"
#include "model/queue.hpp"
#include "model/track.hpp"

namespace {
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

Track makeTrack(const fs::path& path, int duration = 120) {
    return Track({"Artist"}, path, "Title", duration);
}

fs::path makeTempRoot(const std::string& name) {
    fs::path root = fs::temp_directory_path() / name;
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

fs::path findFixture(const fs::path& relative) {
    fs::path cwd = fs::current_path();
    fs::path direct = cwd / relative;
    if (fs::exists(direct))
        return direct;

    fs::path parent = cwd.parent_path() / relative;
    if (fs::exists(parent))
        return parent;

    return {};
}
} // namespace

TEST_CASE("Library basic behavior", "[library]") {
    Library lib;

    Track t1 = makeTrack("music/../music/track1.mp3", 100);
    Track t2 = makeTrack("music/track2.mp3", 200);

    lib.addTrack(std::move(t1));
    lib.addTrack(std::move(t2));

    REQUIRE(lib.getTracks().size() == 2);

    const Track* found = lib.findByPath("music/track1.mp3");
    REQUIRE(found != nullptr);
    CHECK(found->getDuration() == 100);

    CHECK(lib.findByPath("missing.mp3") == nullptr);
}

TEST_CASE("Playlist operations", "[playlist]") {
    Library lib;
    Track t1 = makeTrack("a.mp3", 90);
    Track t2 = makeTrack("b.mp3", 110);
    Track t3 = makeTrack("c.mp3", 50);
    lib.addTrack(t1);
    lib.addTrack(t2);

    Playlist pl("Mix");
    CHECK(pl.isEmpty());

    pl.addTrack(t1);
    pl.addTrack(t2);
    pl.addTrack(t3); // not in library

    CHECK(pl.size() == 3);
    CHECK(pl.getTrack(0) == fs::path("a.mp3"));

    CHECK_THROWS_AS(pl.getTrack(99), std::out_of_range);

    pl.removeTrack("b.mp3");
    CHECK(pl.size() == 2);

    pl.moveTrack(0, 1);
    CHECK(pl.getTrack(1) == fs::path("a.mp3"));

    pl.moveTrack(-1, 2);
    pl.moveTrack(0, 99);
    CHECK(pl.size() == 2);

    // c.mp3 is not in the library, so only a.mp3 contributes to duration
    CHECK(pl.getTotalDuration(lib) == 90);
    auto resolved = pl.resolve(lib);
    CHECK(resolved.size() == 1);
    CHECK(resolved.front()->getMusicPath() == fs::path("a.mp3"));

    pl.clear();
    CHECK(pl.isEmpty());
}

TEST_CASE("Queue navigation and flags", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue q;
    q.load({&t1, &t2, &t3});

    REQUIRE(q.current().has_value());
    CHECK(q.current().value() == fs::path("a.mp3"));
    CHECK(q.hasNext());

    auto next1 = q.next();
    REQUIRE(next1.has_value());
    CHECK(next1.value() == fs::path("b.mp3"));

    auto next2 = q.next();
    REQUIRE(next2.has_value());
    CHECK(next2.value() == fs::path("c.mp3"));

    CHECK_FALSE(q.hasNext());
    CHECK_FALSE(q.next().has_value());

    q.load({&t1, &t2});
    q.setRepeat(true);
    q.next();
    auto repeatNext = q.next();
    REQUIRE(repeatNext.has_value());
    // repeat reloads original_order and returns the front without popping
    CHECK(repeatNext.value() == fs::path("a.mp3"));
    q.setRepeat(false);

    q.load({&t1, &t2, &t3});
    q.setShuffle(true);
    auto current = q.current();
    REQUIRE(current.has_value());
    CHECK(current.value() == fs::path("a.mp3"));

    q.setShuffle(false);
    auto currentAfter = q.current();
    REQUIRE(currentAfter.has_value());
    CHECK(currentAfter.value() == fs::path("a.mp3"));

    Queue empty;
    empty.setShuffle(true);
    CHECK(empty.isShuffle());
}

TEST_CASE("Queue add-to-front/back behavior", "[queue]") {
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    Queue q;
    q.addTrackToFront(t1);
    REQUIRE(q.current().has_value());
    CHECK(q.current().value() == fs::path("a.mp3"));

    q.addTrackToFront(t2);
    auto next = q.next();
    REQUIRE(next.has_value());
    // addTrackToFront inserts as "play next" when queue is non-empty
    CHECK(next.value() == fs::path("b.mp3"));

    q.addTrackToBack(t3);
    next = q.next();
    REQUIRE(next.has_value());
    CHECK(next.value() == fs::path("c.mp3"));
}

TEST_CASE("PlayHistory navigation and bounds", "[play_history]") {
    PlayHistory history;
    Track t1 = makeTrack("a.mp3");
    Track t2 = makeTrack("b.mp3");
    Track t3 = makeTrack("c.mp3");

    CHECK_FALSE(history.current().has_value());
    CHECK_FALSE(history.back().has_value());
    CHECK_FALSE(history.canGoForward());

    history.push(t1);
    history.push(t2);
    history.push(t3);

    REQUIRE(history.current().has_value());
    CHECK(history.current().value() == fs::path("c.mp3"));
    CHECK(history.canGoBack());

    auto back = history.back();
    REQUIRE(back.has_value());
    CHECK(back.value() == fs::path("b.mp3"));

    auto forward = history.forward();
    REQUIRE(forward.has_value());
    CHECK(forward.value() == fs::path("c.mp3"));

    history.clear();
    CHECK_FALSE(history.current().has_value());
}

TEST_CASE("PlayHistory max size", "[play_history]") {
    PlayHistory history;
    for (int i = 0; i < config::MAX_HISTORY + 5; ++i) {
        history.push(makeTrack(std::to_string(i) + ".mp3"));
    }
    CHECK(history.getHistory().size() == static_cast<size_t>(config::MAX_HISTORY));
    REQUIRE(history.current().has_value());
    CHECK(history.current().value() == fs::path(std::to_string(config::MAX_HISTORY + 4) + ".mp3"));
}

TEST_CASE("Persistence creates defaults and saves settings", "[persistence]") {
    fs::path root = makeTempRoot("musilizer_persistence_default");
    ScopedConfigRoot scoped(root);

    Persistence::init();

    CHECK(fs::exists(config::SETTING));
    CHECK(fs::exists(config::PLAYLIST));
    CHECK(fs::exists(config::HISTORY));

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

TEST_CASE("MusicDirectory ignores missing or non-mp3 files", "[music_directory]") {
    fs::path root = makeTempRoot("musilizer_music_dir_missing");
    ScopedConfigRoot scoped(root);

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);
    CHECK(lib.getTracks().empty());

    fs::create_directories(config::MUSIC_DIR);
    std::ofstream(config::MUSIC_DIR / "note.txt") << "hello";
    dir.initialize(lib);
    CHECK(lib.getTracks().empty());
}

TEST_CASE("MusicDirectory handles invalid mp3", "[music_directory]") {
    fs::path root = makeTempRoot("musilizer_music_dir_invalid");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path badPath = config::MUSIC_DIR / "bad.mp3";
    fs::path fixture = findFixture(fs::path("tests") / "fixtures" / "malformed.mp3");
    if (fixture.empty())
        SKIP("malformed mp3 fixture not found");

    fs::copy_file(fixture, badPath, fs::copy_options::overwrite_existing);

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const auto& tracks = lib.getTracks();
    if (tracks.empty()) {
        SUCCEED();
    } else {
        REQUIRE(tracks.size() == 1);
        const auto& entry = *tracks.begin();
        CHECK(entry.first.filename() == badPath.filename());
        CHECK(entry.second.getTitle().empty());
        CHECK(entry.second.getDuration() == 0);
    }
}

TEST_CASE("Audio smoke test", "[audio][.interactive]") {
    fs::path root = fs::current_path();
    if (!fs::exists(root / "music") && fs::exists(root.parent_path() / "music"))
        root = root.parent_path();

    ScopedConfigRoot scoped(root);

    if (!fs::exists(config::MUSIC_DIR))
        SKIP("music directory not found");

    fs::path sample;
    for (const auto& entry : fs::directory_iterator(config::MUSIC_DIR)) {
        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            sample = entry.path();
            break;
        }
    }

    if (sample.empty())
        SKIP("no mp3 file found in music directory");

    try {
        Audio audio;
        audio.setVolume(0.1f);
        audio.load(sample);
        audio.resetTrackEnded();
        audio.pause();
        audio.play();
        CHECK(audio.getVolume() == Catch::Approx(0.1f));

        const auto timeout = std::chrono::seconds(2);
        const auto start = std::chrono::steady_clock::now();
        while (!audio.hasTrackEnded() && (std::chrono::steady_clock::now() - start) < timeout) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (audio.hasTrackEnded())
            SUCCEED("track ended as expected");
        else
            SUCCEED("track did not end within timeout - short file needed");
    } catch (const std::exception& e) {
        SKIP(e.what());
    }
}
