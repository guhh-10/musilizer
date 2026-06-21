#pragma once
#include <vector>
#include <kiss_fftr.h>

class FftAnalyzer {
    private:
        kiss_fftr_cfg             cfg_;

        std::vector<float>           window_;
        std::vector<kiss_fft_scalar> mono_;
        std::vector<kiss_fft_cpx>   cpx_;
        std::vector<float>           magnitude_;
        std::vector<float>           bins_;
        std::vector<float>           peaks_;

    public:
        // Number of interleaved stereo samples consumed per analysis frame.
        // 1024 frames * 2 channels = 2048 floats ≈ 21ms at 48kHz.
        static constexpr int FFT_SIZE      = 1024;
        static constexpr int INPUT_SAMPLES = FFT_SIZE * 2; // stereo

        // Number of output display bars (log-spaced frequency bins).
        static constexpr int NUM_BINS = 64;

        FftAnalyzer();
        ~FftAnalyzer();

        FftAnalyzer(const FftAnalyzer&)            = delete;
        FftAnalyzer& operator=(const FftAnalyzer&) = delete;

        const std::vector<float>& analyze(const float* samples, int count);
        const std::vector<float>& peaks() const { return peaks_; }
        void update(float dt);

        // Rise/fall smoothing factors (0..1). Higher = faster response.
        float alphaRise = 0.8f;
        float alphaFall = 0.15f;

        // How fast peak markers fall (magnitude units per second).
        float peakDecay = 0.4f;
};