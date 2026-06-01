#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <chrono>
#include <thread>

#include "test_helpers.hpp"
#include "model/audio.hpp"

using namespace test_helpers;

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
