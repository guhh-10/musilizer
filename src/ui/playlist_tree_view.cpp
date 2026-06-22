#include <IconsLucide.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "ui/fonts.hpp"
#include "ui/imgui_widgets.hpp"
#include "ui/playlist_tree_view.hpp"

namespace {

bool DrawIconButtonMenuItem(const char* label, bool enabled, float min_width, ImVec4 text_color = ImGui::GetStyle().Colors[ImGuiCol_Text])
{
    bool pressed = false;
    ImGui::BeginDisabled(!enabled);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    if (ImGui::Selectable(label, false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(min_width, 0))) {
        pressed = true;
    }
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
    return pressed;
}

void RenderPlaylistGroupNode(const PlaylistGroup& group, const std::string& base_id, int group_idx, ImGuiID& global_active_id, const PlaylistViewActions& actions)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImGuiStyle& style = g.Style;
    ImDrawList* draw_list = window->DrawList;

    float row_height = 24.0f;
    float header_height = 32.0f;
    float animation_speed = 12.0f;
    float content_width = ImGui::GetContentRegionAvail().x;

    std::string folder_id_str = base_id + "_f_" + std::to_string(group_idx);
    ImGuiID folder_id = window->GetID(folder_id_str.c_str());

    float& f_hover = SmoothAnimState::GetRef(folder_id);
    float& f_active = SmoothAnimState::GetRef(folder_id + 555);
    bool& folder_open = *(bool*)&SmoothAnimState::GetRef(folder_id + 999);

    ImVec2 folder_pos = window->DC.CursorPos;
    ImRect folder_bb(folder_pos, ImVec2(folder_pos.x + content_width, folder_pos.y + header_height));

    ImGui::ItemSize(folder_bb, style.FramePadding.y);
    if (ImGui::ItemAdd(folder_bb, folder_id)) {
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(folder_bb, folder_id, &hovered, &held,
            ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick);

        if (pressed) {
            global_active_id = folder_id;

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (actions.playPlaylist) {
                    actions.playPlaylist(group_idx);
                }
            } else {
                folder_open = !folder_open;
            }
        }

        f_hover = ImClamp(f_hover + (hovered ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);
        f_active = ImClamp(f_active + ((global_active_id == folder_id) ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);

        ImVec4 col_hover = style.Colors[ImGuiCol_HeaderHovered];
        ImVec4 col_active = style.Colors[ImGuiCol_HeaderActive];
        if (col_active.w == 0.0f) col_active = style.Colors[ImGuiCol_SliderGrab];

        float bg_alpha = (col_hover.w * f_hover) * (1.0f - f_active) + (col_active.w * 0.4f) * f_active;
        if (bg_alpha > 0.01f) {
            ImVec4 mixed_bg = ImVec4(
                col_hover.x + (col_active.x - col_hover.x) * f_active,
                col_hover.y + (col_active.y - col_hover.y) * f_active,
                col_hover.z + (col_active.z - col_hover.z) * f_active,
                bg_alpha
            );
            draw_list->AddRectFilled(folder_bb.Min, folder_bb.Max, ImGui::GetColorU32(mixed_bg), 3.0f);
        }

        float center_y = folder_bb.Min.y + (header_height * 0.5f);
        float text_offset_y = center_y - (ImGui::GetTextLineHeight() * 0.5f);
        float x_cursor = folder_bb.Min.x + 8.0f;

        ImVec4 col_text_normal = style.Colors[ImGuiCol_Text];
        ImVec4 col_text_accent = style.Colors[ImGuiCol_SliderGrabActive];

        ImU32 icon_color = ImGui::GetColorU32(ImVec4(
            col_text_normal.x + (col_text_accent.x - col_text_normal.x) * f_active,
            col_text_normal.y + (col_text_accent.y - col_text_normal.y) * f_active,
            col_text_normal.z + (col_text_accent.z - col_text_normal.z) * f_active,
            1.0f
        ));

        const char* current_icon = folder_open ? ICON_LC_LIST_MUSIC : ICON_LC_LIST_TREE;
        ImGui::PushFont(FontManager::icons());
        draw_list->AddText(ImVec2(x_cursor, center_y - (ImGui::GetTextLineHeight() * 0.5f)), icon_color, current_icon);
        float icon_width = ImGui::CalcTextSize(current_icon).x;
        ImGui::PopFont();
        x_cursor += icon_width + 8.0f;

        std::string count_str = std::to_string(group.tracks.size()) + (group.tracks.size() == 1 ? " track" : " tracks");
        ImVec2 count_text_size = ImGui::CalcTextSize(count_str.c_str());
        float count_x_pos = folder_bb.Max.x - count_text_size.x - 8.0f;
        draw_list->AddText(ImVec2(count_x_pos, text_offset_y), ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]), count_str.c_str());

        float max_title_width = count_x_pos - x_cursor - 8.0f;
        if (max_title_width > 0.0f) {
            ImVec2 text_min = ImVec2(x_cursor, folder_bb.Min.y);
            ImVec2 text_max = ImVec2(x_cursor + max_title_width, folder_bb.Max.y);
            ImGui::PushStyleColor(ImGuiCol_Text, icon_color);
            ImGui::RenderTextClipped(text_min, text_max, group.name.c_str(), nullptr, nullptr, ImVec2(0.0f, 0.5f));
            ImGui::PopStyleColor();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 6.0f));
        if (ImGui::BeginPopupContextItem(folder_id_str.c_str())) {
            const char* opt1 = " Play Entire Playlist Selection";
            const char* opt2 = " Delete Playlist";
            float folder_auto_width = ImMax(ImGui::CalcTextSize(opt1).x, ImGui::CalcTextSize(opt2).x) + (style.ItemInnerSpacing.x * 2.0f);
            if (DrawIconButtonMenuItem(opt1, actions.playPlaylist != nullptr, folder_auto_width)) {
                if (actions.playPlaylist) {
                    actions.playPlaylist(group_idx);
                }
            }
            if (DrawIconButtonMenuItem(opt2, actions.removePlaylist != nullptr, folder_auto_width, ImVec4(0.75f, 0.21f, 0.16f, 1.0f))) {
                if (actions.removePlaylist) {
                    actions.removePlaylist(group.name);
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
    }

    if (folder_open) {
        for (int t = 0; t < (int)group.tracks.size(); t++) {
            const auto& track = group.tracks[t];
            std::string track_id_str = folder_id_str + "_t_" + std::to_string(t);
            ImGuiID track_id = window->GetID(track_id_str.c_str());

            float& t_hover = SmoothAnimState::GetRef(track_id);

            ImVec2 track_pos = window->DC.CursorPos;
            ImRect track_bb(track_pos, ImVec2(track_pos.x + content_width, track_pos.y + row_height));

            ImGui::ItemSize(track_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(track_bb, track_id)) continue;

            bool t_hovered, t_held;
            bool t_pressed = ImGui::ButtonBehavior(track_bb, track_id, &t_hovered, &t_held,
                ImGuiButtonFlags_PressedOnClickRelease);

            if (t_pressed && actions.playTrack) {
                actions.playTrack(group_idx, t);
            }

            t_hover = ImClamp(t_hover + (t_hovered ? g.IO.DeltaTime : -g.IO.DeltaTime) * animation_speed, 0.0f, 1.0f);

            float t_content_start_x = track_bb.Min.x + 22.0f;
            if (t_hover > 0.01f) {
                ImVec4 col_h = style.Colors[ImGuiCol_HeaderHovered];
                draw_list->AddRectFilled(ImVec2(t_content_start_x - 4.0f, track_bb.Min.y), track_bb.Max, ImGui::GetColorU32(ImVec4(col_h.x, col_h.y, col_h.z, col_h.w * t_hover)), 3.0f);
            }

            float t_center_y = track_bb.Min.y + (row_height * 0.5f);
            float t_text_offset_y = t_center_y - (ImGui::GetTextLineHeight() * 0.5f);
            float t_x_cursor = t_content_start_x;

            ImU32 track_icon_col = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);
            draw_list->AddRectFilled(ImVec2(t_x_cursor, t_center_y - 4.0f), ImVec2(t_x_cursor + 2.0f, t_center_y + 4.0f), track_icon_col);
            draw_list->AddRectFilled(ImVec2(t_x_cursor + 4.0f, t_center_y - 6.0f), ImVec2(t_x_cursor + 6.0f, t_center_y + 6.0f), track_icon_col);
            draw_list->AddRectFilled(ImVec2(t_x_cursor + 8.0f, t_center_y - 2.0f), ImVec2(t_x_cursor + 10.0f, t_center_y + 2.0f), track_icon_col);
            t_x_cursor += 18.0f;

            float max_track_title_width = track_bb.Max.x - t_x_cursor - 8.0f;
            if (!track.duration.empty()) {
                ImVec2 ts = ImGui::CalcTextSize(track.duration.c_str());
                max_track_title_width -= (ts.x + 8.0f);
                draw_list->AddText(ImVec2(track_bb.Max.x - ts.x - 8.0f, t_text_offset_y), ImGui::GetColorU32(ImGuiCol_TextDisabled), track.duration.c_str());
            }

            if (max_track_title_width > 0.0f) {
                ImVec4 col_muted = style.Colors[ImGuiCol_TextDisabled];
                ImVec4 col_normal = style.Colors[ImGuiCol_Text];
                ImVec4 mixed_track_color = ImVec4(
                    col_muted.x + (col_normal.x - col_muted.x) * t_hover,
                    col_muted.y + (col_normal.y - col_muted.y) * t_hover,
                    col_muted.z + (col_normal.z - col_muted.z) * t_hover,
                    col_muted.w + (col_normal.w - col_muted.w) * t_hover
                );
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(mixed_track_color));
                ImGui::RenderTextClipped(ImVec2(t_x_cursor, track_bb.Min.y), ImVec2(t_x_cursor + max_track_title_width, track_bb.Max.y), track.name.c_str(), nullptr, nullptr, ImVec2(0.0f, 0.5f));
                ImGui::PopStyleColor();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 6.0f));
            if (ImGui::BeginPopupContextItem(track_id_str.c_str())) {
                const char* topt1 = " Move Up";
                const char* topt2 = " Move Down";
                const char* topt3 = " Delete from Playlist";
                float track_auto_width = ImMax(ImGui::CalcTextSize(topt1).x, ImMax(ImGui::CalcTextSize(topt2).x, ImGui::CalcTextSize(topt3).x)) + (style.ItemInnerSpacing.x * 2.0f);

                if (DrawIconButtonMenuItem(topt1, actions.moveTrack != nullptr && t > 0, track_auto_width) && actions.moveTrack) {
                    actions.moveTrack(group.name, t, t - 1);
                }
                if (DrawIconButtonMenuItem(topt2, actions.moveTrack != nullptr && t < (int)group.tracks.size() - 1, track_auto_width) && actions.moveTrack) {
                    actions.moveTrack(group.name, t, t + 1);
                }

                if (DrawIconButtonMenuItem(topt3, actions.removeTrack != nullptr, track_auto_width, ImVec4(0.75f, 0.21f, 0.16f, 1.0f))) {
                    if (actions.removeTrack && actions.getPlaylists) {
                        const auto& playlists = actions.getPlaylists();
                        if (group_idx >= 0 && group_idx < (int)playlists.size()) {
                            const auto& backendTracks = playlists[group_idx].getPlaylistTracks();
                            if (t >= 0 && t < (int)backendTracks.size()) {
                                actions.removeTrack(group.name, backendTracks[t]);
                            }
                        }
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleVar(2);
        }
    }
}

} // namespace

void StrictTwoTierPlaylistView(const char* str_id, const std::vector<PlaylistGroup>& list, const PlaylistViewActions& actions)
{
    static ImGuiID global_active_playlist_folder_id = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    for (int i = 0; i < (int)list.size(); i++) {
        RenderPlaylistGroupNode(list[i], str_id, i, global_active_playlist_folder_id, actions);
    }
    ImGui::PopStyleVar();
}
