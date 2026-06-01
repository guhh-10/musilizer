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
