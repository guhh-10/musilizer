#include <iostream>
#include <thread>
#include <chrono>
#include <config.hpp>

#include "model/library.hpp"
#include "model/musicDirectory.hpp"
#include "model/track.hpp"
#include "model/audio.hpp"
#include "model/queue.hpp"
#include "model/playHistory.hpp"

int main(int argc, char* argv[]) {
    fs::path binaryDir = fs::weakly_canonical(argv[0]).parent_path();
    config::init(binaryDir);
}