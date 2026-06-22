#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

#include "ui/waveform_panel.hpp"

WaveformPanel::WaveformPanel(Player &player)
    : player_(player), sample_buf_(SAMPLE_COUNT, 0.0f) {}

void WaveformPanel::draw() {
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  if (canvas_size.x < 1.0f || canvas_size.y < 1.0f)
    return;

  // ── 1. Pull real audio samples from the ring buffer ───────────────────────
  player_.getSamples(sample_buf_.data(), SAMPLE_COUNT);

  // ── 2. Run FFT and get the smoothed magnitude bins ────────────────────────
  const std::vector<float> &bins =
      analyzer_.analyze(sample_buf_.data(), SAMPLE_COUNT);

  // ── 3. Fluid Grid Layout ───────────────────────────────────────────────────
  const float spacing   = FftAnalyzer::barGapFromFftSize();
  const float bar_width = FftAnalyzer::barWidthFromFftSize();

  int num_bars =
      static_cast<int>((canvas_size.x + spacing) / (bar_width + spacing));
  num_bars = std::clamp(num_bars, 1, FftAnalyzer::NUM_BINS);

  if (smoothed_heights_.size() != static_cast<size_t>(num_bars)) {
    smoothed_heights_.resize(num_bars, 0.0f);
  }

  const int src_bins = static_cast<int>(bins.size());

  for (int i = 0; i < num_bars; ++i) {
    int src = (src_bins > 1 && num_bars > 1)
        ? static_cast<int>(static_cast<float>(i) * (src_bins - 1) / (num_bars - 1) + 0.5f)
        : 0;
    src = std::clamp(src, 0, src_bins - 1);
    smoothed_heights_[i] = std::clamp(bins[src], 0.0f, 1.0f);
  }

  const ImGuiStyle &style = ImGui::GetStyle();

  // Theme colors for the lines
  const ImVec4 &c_bass = style.Colors[ImGuiCol_SliderGrabActive];
  const ImVec4 &c_mid  = style.Colors[ImGuiCol_PlotLinesHovered];
  const ImVec4 &c_high = style.Colors[ImGuiCol_TextDisabled];

  const float y_center  = canvas_pos.y + canvas_size.y * 0.5f;
  const float max_half_h = canvas_size.y * 0.5f - 10.0f;

  for (int i = 0; i < num_bars; ++i) {
    if (smoothed_heights_[i] < 0.001f)
      continue;

    const float x_pos =
        canvas_pos.x + spacing + i * (bar_width + spacing) + (bar_width * 0.5f);
    const float half_h = smoothed_heights_[i] * max_half_h;

    // ── Color Interpolation ───────────────────────────────────────────────
    const float progress =
        static_cast<float>(i) / (num_bars > 1 ? num_bars - 1 : 1);
    ImVec4 line_color_vec;
    if (progress < 0.5f) {
      const float t = progress / 0.5f;
      line_color_vec = ImVec4(c_bass.x + (c_mid.x - c_bass.x) * t,
                              c_bass.y + (c_mid.y - c_bass.y) * t,
                              c_bass.z + (c_mid.z - c_bass.z) * t, 1.0f);
    } else {
      const float t = (progress - 0.5f) / 0.5f;
      line_color_vec =
          ImVec4(c_mid.x + (c_high.x - c_mid.x) * t,
                 c_mid.y + (c_high.y - c_mid.y) * t,
                 c_mid.z + (c_high.z - c_mid.z) * t, 1.0f - t * 0.3f);
    }
    const ImU32 line_color = ImGui::ColorConvertFloat4ToU32(line_color_vec);

    draw_list->AddLine(ImVec2(x_pos, y_center - half_h),
                       ImVec2(x_pos, y_center + half_h),
                       line_color,
                       bar_width);
  }

  ImGui::Dummy(canvas_size);
}