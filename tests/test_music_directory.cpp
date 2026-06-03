#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "test_helpers.hpp"
#include "model/library.hpp"
#include "model/music_directory.hpp"

using namespace test_helpers;

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

TEST_CASE("MusicDirectory reads metadata", "[music_directory]") {
    fs::path root = makeTempRoot("musilizer_music_dir_metadata");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path topLevel = config::MUSIC_DIR / "sample.mp3";
    writeSampleMp3(topLevel);
    tagSampleMp3(topLevel, "Sample Title", "Sample Artist");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(topLevel);
    REQUIRE(track != nullptr);
    CHECK(track->getTitle() == "Sample Title");
    REQUIRE(track->getArtists().size() == 1);
    CHECK(track->getArtists().front() == "Sample Artist");
    CHECK(track->getDuration() > 0);
}

TEST_CASE("MusicDirectory recurses into nested folders", "[music_directory]") {
    fs::path root = makeTempRoot("musilizer_music_dir_recurse");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path topLevel = config::MUSIC_DIR / "sample.mp3";
    writeSampleMp3(topLevel);
    tagSampleMp3(topLevel, "Sample Title", "Sample Artist");

    fs::path nestedDir = config::MUSIC_DIR / "deep" / "more";
    fs::create_directories(nestedDir);
    fs::path nested = nestedDir / "nested.mp3";
    fs::copy_file(topLevel, nested, fs::copy_options::overwrite_existing);

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    CHECK(lib.findByPath(nested) != nullptr);
}

TEST_CASE("MusicDirectory reads single genre", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_single");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "single.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    tagSampleMp3Genre(mp3, "Rock");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    REQUIRE(track->getGenres().size() == 1);
    CHECK(track->getGenres().front() == "Rock");
}

TEST_CASE("MusicDirectory splits multiple genres on semicolon", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_multi");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "multi.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    tagSampleMp3Genre(mp3, "Rock;Alternative;Indie");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    REQUIRE(track->getGenres().size() == 3);
    CHECK(track->getGenres()[0] == "Rock");
    CHECK(track->getGenres()[1] == "Alternative");
    CHECK(track->getGenres()[2] == "Indie");
}

TEST_CASE("MusicDirectory trims whitespace around genre tokens", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_trim");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "trim.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    tagSampleMp3Genre(mp3, "  Rock ; Alternative ;  Indie  ");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    REQUIRE(track->getGenres().size() == 3);
    CHECK(track->getGenres()[0] == "Rock");
    CHECK(track->getGenres()[1] == "Alternative");
    CHECK(track->getGenres()[2] == "Indie");
}

TEST_CASE("MusicDirectory drops empty genre tokens from stray delimiters", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_empty_token");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "stray.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    tagSampleMp3Genre(mp3, "Rock;;Jazz;");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    // Only "Rock" and "Jazz" — two empty tokens are discarded.
    REQUIRE(track->getGenres().size() == 2);
    CHECK(track->getGenres()[0] == "Rock");
    CHECK(track->getGenres()[1] == "Jazz");
}

TEST_CASE("MusicDirectory produces empty genre list when TCON absent", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_absent");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "notag.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    // Deliberately no tagSampleMp3Genre call.

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    CHECK(track->getGenres().empty());
}

TEST_CASE("MusicDirectory preserves genre casing as stored in tag", "[music_directory][genre]") {
    fs::path root = makeTempRoot("musilizer_music_dir_genre_case");
    ScopedConfigRoot scoped(root);

    fs::create_directories(config::MUSIC_DIR);
    fs::path mp3 = config::MUSIC_DIR / "case.mp3";
    writeSampleMp3(mp3);
    tagSampleMp3(mp3, "Title", "Artist");
    tagSampleMp3Genre(mp3, "Trip-Hop;R&B");

    Library lib;
    MusicDirectory dir;
    dir.initialize(lib);

    const Track* track = lib.findByPath(mp3);
    REQUIRE(track != nullptr);
    REQUIRE(track->getGenres().size() == 2);
    // Casing must be preserved — normalisation happens inside Search/Recommender.
    CHECK(track->getGenres()[0] == "Trip-Hop");
    CHECK(track->getGenres()[1] == "R&B");
}