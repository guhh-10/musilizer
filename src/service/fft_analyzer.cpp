#include "service/fft_analyzer.hpp"

#include <algorithm>
#include <cmath>

#ifndef M_PI
static constexpr float M_PI_F = 3.14159265358979323846f;
#else
static constexpr float M_PI_F = static_cast<float>(M_PI);
#endif

FftAnalyzer::FftAnalyzer() {
  cfg_ = kiss_fftr_alloc(FFT_SIZE, /*inverse=*/0, nullptr, nullptr);

  window_.resize(FFT_SIZE);
  for (int i = 0; i < FFT_SIZE; ++i)
    window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI_F * i / (FFT_SIZE - 1)));

  mono_.resize(FFT_SIZE, 0.0f);
  cpx_.resize(FFT_SIZE / 2 + 1);
  magnitude_.resize(FFT_SIZE / 2, 0.0f);
  bins_.resize(NUM_BINS, 0.0f);
  peaks_.resize(NUM_BINS, 0.0f);
}

FftAnalyzer::~FftAnalyzer() { kiss_fftr_free(cfg_); }

const std::vector<float> &FftAnalyzer::analyze(const float *samples,
                                               int count) {
  int frames = std::min(count / 2, FFT_SIZE); // stereo pairs → mono frames

  for (int i = 0; i < FFT_SIZE; ++i) {
    if (i < frames) {
      float mono = (samples[i * 2] + samples[i * 2 + 1]) * 0.5f;
      mono_[i] = mono * window_[i];
    } else {
      mono_[i] = 0.0f; // zero-pad if short
    }
  }

  kiss_fftr(cfg_, mono_.data(), cpx_.data());

  // Normalise so a full-scale sine ≈ 1.0
  // We use a base scale, and we'll apply dynamic range compression below.
  float scale = (2.0f / FFT_SIZE) * 2.0f;
  for (int i = 0; i < FFT_SIZE / 2; ++i) {
    float r = cpx_[i].r * scale;
    float im = cpx_[i].i * scale;
    float mag = std::sqrt(r * r + im * im);

    // Dynamic range compression: taking the square root of the magnitude
    // makes quieter sounds more visible, making the visualizer feel more
    // "alive".
    mag = std::sqrt(mag);

    // High frequency boost: music has less energy in the highs (pink noise
    // curve). This boosts the higher bins so they aren't completely flat
    // compared to the bass.
    float freq_boost = 1.0f + (static_cast<float>(i) / (FFT_SIZE / 2)) * 2.5f;

    // The final multiplier here (2.0f) is the overall sensitivity.
    // Increase this number if the bars are still too low, or decrease if they
    // clip at the top!
    magnitude_[i] = mag * freq_boost * 3.0f;
  }

  const int magBins = FFT_SIZE / 2;
  const float logLo = std::log2(1.0f);                            // bin 1
  const float logHi = std::log2(static_cast<float>(magBins - 1)); // bin 511

  for (int b = 0; b < NUM_BINS; ++b) {
    float t0 = static_cast<float>(b) / NUM_BINS;
    float t1 = static_cast<float>(b + 1) / NUM_BINS;

    int lo = static_cast<int>(std::pow(2.0f, logLo + t0 * (logHi - logLo)));
    int hi = static_cast<int>(std::pow(2.0f, logLo + t1 * (logHi - logLo)));
    lo = std::clamp(lo, 1, magBins - 1);
    hi = std::clamp(hi, lo + 1, magBins);

    float peak = 0.0f;
    for (int i = lo; i < hi; ++i)
      peak = std::max(peak, magnitude_[i]);

    float alpha = (peak > bins_[b]) ? alphaRise : alphaFall;
    bins_[b] = alpha * peak + (1.0f - alpha) * bins_[b];

    if (bins_[b] > peaks_[b])
      peaks_[b] = bins_[b];
  }

  return bins_;
}

void FftAnalyzer::update(float dt) {
  float drop = peakDecay * dt;
  for (float &p : peaks_)
    p = std::max(0.0f, p - drop);
}