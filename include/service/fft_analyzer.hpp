// fft_analyzer.hpp
#pragma once
#include <kiss_fftr.h>
#include <vector>

// Compile-time log2 for power-of-two values.
  static constexpr int log2_ct(int n) { return n <= 1 ? 0 : 1 + log2_ct(n / 2); }

class FftAnalyzer {
private:
  kiss_fftr_cfg cfg_;

  std::vector<float> window_;
  std::vector<kiss_fft_scalar> mono_;
  std::vector<kiss_fft_cpx> cpx_;
  std::vector<float> magnitude_;
  std::vector<float> bins_;

public:
  static constexpr int FFT_SIZE = 1024;
  static constexpr int INPUT_SAMPLES = FFT_SIZE * 2; // stereo

  static constexpr int NUM_BINS = log2_ct(FFT_SIZE / 2) * 16;

  FftAnalyzer();
  ~FftAnalyzer();

  FftAnalyzer(const FftAnalyzer &) = delete;
  FftAnalyzer &operator=(const FftAnalyzer &) = delete;

  const std::vector<float> &analyze(const float *samples, int count);

  float smoothingTimeConstant = 0.65f;

  static constexpr float barWidthFromFftSize() {
    constexpr float REF_WIDTH = 4.0f;
    constexpr int   REF_SIZE  = 1024;
    constexpr int ratio = FFT_SIZE / REF_SIZE;
    constexpr int halvings = (ratio >= 1)
        ? (ratio == 1 ? 0 : ratio == 2 ? 1 : ratio == 4 ? 2 : ratio == 8 ? 3 : 4)
        : 0;
    constexpr float w = REF_WIDTH / (1 << halvings);
    return w < 1.0f ? 1.0f : w;
  }

  static constexpr float barGapFromFftSize() {
    constexpr float REF_GAP  = 2.0f;
    constexpr int   REF_SIZE = 1024;
    constexpr int ratio    = FFT_SIZE / REF_SIZE;
    constexpr int halvings = (ratio >= 1)
        ? (ratio == 1 ? 0 : ratio == 2 ? 1 : ratio == 4 ? 2 : ratio == 8 ? 3 : 4)
        : 0;
    constexpr float g = REF_GAP / (1 << halvings);
    return g < 1.0f ? 1.0f : g;
  }
};