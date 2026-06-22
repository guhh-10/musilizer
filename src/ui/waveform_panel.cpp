#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

#include "ui/waveform_panel.hpp"

WaveformPanel::WaveformPanel(Player &player)
    : player_(player), smoothed_heights_(NUM_BARS, 0.0f),
      sample_buf_(SAMPLE_COUNT, 0.0f) {}

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

  analyzer_.update(ImGui::GetIO().DeltaTime);

  // ── 3. Map FftAnalyzer bins → NUM_BARS display bars ───────────────────────
  const int src_bins = static_cast<int>(bins.size()); // 64

  for (int i = 0; i < NUM_BARS; ++i) {
    int src = static_cast<int>(static_cast<float>(i) / NUM_BARS * src_bins);
    src = std::clamp(src, 0, src_bins - 1);
    smoothed_heights_[i] = std::clamp(bins[src], 0.0f, 1.0f);
  }

  const ImGuiStyle &style = ImGui::GetStyle();

  // Theme colors for the lines
  const ImVec4 &c_bass = style.Colors[ImGuiCol_SliderGrabActive]; 
  const ImVec4 &c_mid = style.Colors[ImGuiCol_PlotLinesHovered];  
  const ImVec4 &c_high = style.Colors[ImGuiCol_TextDisabled];     

  const float spacing = 2.0f;
  const float bar_width = (canvas_size.x - spacing * (NUM_BARS + 1)) / NUM_BARS;
  const float y_center = canvas_pos.y + canvas_size.y * 0.5f;
  const float max_half_h = canvas_size.y * 0.5f - 10.0f;

  for (int i = 0; i < NUM_BARS; ++i) {
    if (smoothed_heights_[i] < 0.001f)
      continue;

    // Calculate the horizontal center of the bar line
    const float x_pos = canvas_pos.x + spacing + i * (bar_width + spacing) + (bar_width * 0.5f);
    const float half_h = smoothed_heights_[i] * max_half_h;

    // ── Color Interpolation: bass → mid → high across bar index ───────────
    const float progress = static_cast<float>(i) / (NUM_BARS - 1);
    ImVec4 line_color_vec;
    if (progress < 0.5f) {
      const float t = progress / 0.5f;
      line_color_vec = ImVec4(c_bass.x + (c_mid.x - c_bass.x) * t,
                              c_bass.y + (c_mid.y - c_bass.y) * t,
                              c_bass.z + (c_mid.z - c_bass.z) * t, 1.0f);
    } else {
      const float t = (progress - 0.5f) / 0.5f;
      line_color_vec = ImVec4(c_mid.x + (c_high.x - c_mid.x) * t,
                              c_mid.y + (c_high.y - c_mid.y) * t,
                              c_mid.z + (c_high.z - c_mid.z) * t,
                              1.0f - t * 0.3f);
    }
    const ImU32 line_color = ImGui::ColorConvertFloat4ToU32(line_color_vec);

    // ── Draw the solid line (mirrored around the center Y anchor) ─────────
    draw_list->AddLine(
        ImVec2(x_pos, y_center - half_h), // Top point
        ImVec2(x_pos, y_center + half_h), // Bottom point
        line_color,
        bar_width                         // Thick enough to fill the bar area
    );
  }

  ImGui::Dummy(canvas_size);
}