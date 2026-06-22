#pragma once

#include <vector>

#include "controller/player.hpp"
#include "service/fft_analyzer.hpp"

class WaveformPanel {
public:
    explicit WaveformPanel(Player& player);

    void draw();

private:
    Player&      player_;
    FftAnalyzer  analyzer_;

    std::vector<float> smoothed_heights_;

    std::vector<float> sample_buf_;

    static constexpr int SAMPLE_COUNT = FftAnalyzer::INPUT_SAMPLES; // 2048 stereo floats
};