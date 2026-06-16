#include <imgui.h>
#include <imgui_internal.h>
#include <IconsLucide.h>

#include "ui/library_panel.hpp"
#include "ui/fonts.hpp"
#include "ui/imgui_widgets.hpp"
#include <vector>
#include <string>

// Inline local mock structure to keep compilation fully safe and static
struct StaticTrack {
    std::string title;
    std::vector<std::string> artists;
    int duration;
    std::string filename;
};

LibraryPanel::LibraryPanel(Player& player, SearchController& search)
    : player_(player)
    , search_(search)
{
    // Initial population — show everything on startup.
    runSearch();
}

void LibraryPanel::runSearch() {
    results_ = search_.query(searchBuf_);
}

void LibraryPanel::draw() {
    ImGuiStyle& style = ImGui::GetStyle();
    float padding = 20.0f; // Symmetrical padding for both left and right sides

    // 1. Enforce Left Padding Explicitly
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);

    // 2. Left Aligned Text with npTitle Font
    ImGui::PushFont(FontManager::npTitle());
    ImGui::Text("Library");
    ImGui::PopFont();
    ImGui::SameLine();

    // 4. Calculate position for Right Aligned Text
    ImGui::PushFont(FontManager::npArtist());
    float right_text_width = ImGui::CalcTextSize("Track Number").x; 
    ImGui::PopFont();

    // Work available width minus the right-side padding and the text itself
    float target_pos_x = ImGui::GetWindowContentRegionMax().x - right_text_width - padding;

    // Prevent overlap if the window gets squeezed too small
    if (target_pos_x < ImGui::GetCursorPosX()) {
        target_pos_x = ImGui::GetCursorPosX(); 
    }

    // Push the cursor to the calculated right position
    ImGui::SetCursorPosX(target_pos_x);

    // 5. Right Aligned Text with npArtist Font (Muted Color)
    ImGui::PushFont(FontManager::npArtist());
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::Text("Track Number");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    // 6. Search Input Bar
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (!window->SkipItems)
        {
            const ImGuiID id = window->GetID("##library_search");

            float width_arg   = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - (padding * 2.0f);
            float padding_x   = 8.0f;
            float padding_y   = 6.0f;
            float input_height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2.0f;
            float total_height = input_height + (padding_y * 2.0f);

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
            ImVec2 pos = window->DC.CursorPos;
            ImRect panel_bb(pos, ImVec2(pos.x + width_arg, pos.y + total_height));

            ImGui::ItemSize(panel_bb, style.FramePadding.y);
            if (ImGui::ItemAdd(panel_bb, id))
            {
                // Silver background panel
                ImU32 white_bg_col     = ImGui::GetColorU32(style.Colors[ImGuiCol_ChildBg]); 
                ImU32 darker_white_border_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Border]);
                float panel_rounding    = 6.0f;

                window->DrawList->AddRectFilled(panel_bb.Min, panel_bb.Max, white_bg_col, panel_rounding);
                window->DrawList->AddRect(panel_bb.Min, panel_bb.Max, darker_white_border_col, panel_rounding, 0, 1.5f);

                // Procedural magnifying glass icon
                float center_y   = panel_bb.Min.y + (total_height * 0.5f);
                ImVec2 icon_center = ImVec2(panel_bb.Min.x + padding_x + 6.0f, center_y - 2.0f);
                float icon_radius  = 4.5f;
                ImU32 icon_color = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);

                window->DrawList->AddCircle(icon_center, icon_radius, icon_color, 16, 2.0f);
                ImVec2 handle_start = ImVec2(icon_center.x + 3.0f, icon_center.y + 3.0f);
                ImVec2 handle_end   = ImVec2(icon_center.x + 8.0f, icon_center.y + 8.0f);
                window->DrawList->AddLine(handle_start, handle_end, icon_color, 2.5f);

                // Embedded text input
                float icon_allocated_width = 24.0f;
                float embedded_input_width = width_arg - (padding_x * 2.0f) - icon_allocated_width;
                window->DC.CursorPos = ImVec2(panel_bb.Min.x + padding_x + icon_allocated_width, panel_bb.Min.y + padding_y);

                ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Text]);
                ImGui::PushStyleColor(ImGuiCol_Text,    style.Colors[ImGuiCol_FrameBgActive]);   

                if (SmoothActiveInputText("##library_search", searchBuf_, sizeof(searchBuf_), ImVec2(embedded_input_width, 0)))
                    runSearch();

                // Draw placeholder hint when empty and not focused
                if (searchBuf_[0] == '\0') {
                    ImVec2 hint_pos = ImVec2(
                        panel_bb.Min.x + padding_x + icon_allocated_width + style.FramePadding.x,
                        panel_bb.Min.y + padding_y + style.FramePadding.y
                    );
                    window->DrawList->AddText(hint_pos, ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]), "Search tracks, artists...");
                }

                ImGui::PopStyleColor(2);

                window->DC.CursorPos = ImVec2(panel_bb.Min.x, panel_bb.Max.y + style.ItemSpacing.y);
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    // ============================================================================
    // WIDGET 6: STATIC INTEGRATED MEDIA TRACK LIST TABLE
    // ============================================================================
    
    // Updated flags: Removed ImGuiTableFlags_RowBg to allow color custom overrides
    ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_NoPadOuterX;

    // Calculate full content width without deducting 'padding * 2.0f'
    float table_width = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    ImVec2 outer_size = ImVec2(table_width, 0.0f); 

    // FORCE THE TABLE CONTEXT BACKGROUND TO MATCH ImGuiCol_Border PERFECTLY
    ImGui::PushStyleColor(ImGuiCol_ChildBg, style.Colors[ImGuiCol_Border]);

    if (ImGui::BeginTable("##static_library_table", 4, table_flags, outer_size))
    {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.0f);
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch, 50.0f);
        ImGui::TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthStretch, 35.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch, 15.0f);

        const float right_padding = 6.0f; 

        // Header Styling Configuration
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

        // Column 0 Header: Manual Right Alignment
        ImGui::TableSetColumnIndex(0);
        {
            ImVec2 cell_pos = ImGui::GetCursorScreenPos();
            float column_width = ImGui::GetContentRegionAvail().x;
            ImVec2 text_size = ImGui::CalcTextSize("#");
            
            float right_edge_x = cell_pos.x + column_width - right_padding;
            ImGui::GetWindowDrawList()->AddText(ImVec2(right_edge_x - text_size.x, cell_pos.y), 
                                                ImGui::GetColorU32(ImGuiCol_Text), "#");
            ImGui::Dummy(ImVec2(0, text_size.y)); 
        }

        // Columns 1-3 Headers: Native Alignment
        ImGui::TableSetColumnIndex(1); ImGui::TableHeader("Title");
        ImGui::TableSetColumnIndex(2); ImGui::TableHeader("Artist");
        ImGui::TableSetColumnIndex(3); ImGui::TableHeader("Duration");

        ImGui::PopStyleColor(3);

        // Fully hardcoded static data matrix completely independent of Player / DB engines
        static const std::vector<StaticTrack> static_tracks = {
            {"Starlight Express", {"Neon Horizon", "Lumina"}, 224, "starlight_express.mp4"},
            {"Midnight Coffee Run", {"The Caffeine Code"}, 185, "midnight_coffee.wav"},
            {"Cybernetic Dreams", {"Glitch Overlord"}, 312, "cyber dreams.mp3"},
            {"Whispering Shadows", {"Echo Location", "Acoustic Mirage"}, 274, "whispering_shadows.flac"},
            {"Solar Wind Flares", {"Cosmic Radiation"}, 199, "solar_flares.ogg"}
        };

        // UI state tracking simulation purely for layout previewing
        static int selected_mock_idx = -1;

        int row_number = 0;
        for (const auto& track : static_tracks) {
            row_number++;
            ImGui::TableNextRow();

            bool is_active = (selected_mock_idx == row_number);

            // Column 0: Index and Play-arrow hover transition state
            ImGui::TableSetColumnIndex(0);

            ImVec2 cell_pos = ImGui::GetCursorScreenPos();
            float column_width = ImGui::GetContentRegionAvail().x;
            float line_height = ImGui::GetTextLineHeight();

            std::string row_select_id = "##static_row_" + std::to_string(row_number);

            if (ImGui::Selectable(row_select_id.c_str(), is_active, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_mock_idx = row_number;
            }

            bool row_hovered = ImGui::IsItemHovered();

            // Right-Aligned Custom Index or Triangle Drawing Pass
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float right_edge_x = cell_pos.x + column_width - right_padding;
            float center_y = cell_pos.y + line_height * 0.5f;

            if (row_hovered || is_active) {
                float tri_size = line_height * 0.55f;
                float tri_width = tri_size * 0.86f;

                ImVec2 p1(right_edge_x - tri_width, center_y - tri_size * 0.5f);
                ImVec2 p2(right_edge_x - tri_width, center_y + tri_size * 0.5f);
                ImVec2 p3(right_edge_x, center_y);

                ImU32 icon_color = is_active ? ImGui::GetColorU32(ImGuiCol_SliderGrabActive) : ImGui::GetColorU32(ImGuiCol_Text);
                draw_list->AddTriangleFilled(p1, p2, p3, icon_color);
            } else {
                char number_buf[8];
                snprintf(number_buf, sizeof(number_buf), "%d", row_number);

                ImVec2 text_size = ImGui::CalcTextSize(number_buf);
                ImVec2 text_pos(right_edge_x - text_size.x, cell_pos.y);

                draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), number_buf);
            }

            // Column 1: Title
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(track.title.c_str());

            // Column 2: Artist List Flattening
            ImGui::TableSetColumnIndex(2);
            std::string artist_str;
            for (size_t i = 0; i < track.artists.size(); ++i) {
                if (i > 0) artist_str += ", ";
                artist_str += track.artists[i];
            }
            ImGui::TextUnformatted(artist_str.c_str());

            // Column 3: Duration format conversion
            ImGui::TableSetColumnIndex(3);
            int min = track.duration / 60;
            int sec = track.duration % 60;
            ImGui::Text("%d:%02d", min, sec);
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleColor(); // Balance the forced background modification
}

// void LibraryPanel::drawContextMenu(const Track& track) {
//     // if (!ImGui::BeginPopupContextItem()) return;

//     // if (ImGui::MenuItem("Play"))
//     //     player_.play(track);

//     // if (ImGui::MenuItem("Queue Next"))
//     //     player_.queueNext(track);

//     // if (ImGui::MenuItem("Queue Last"))
//     //     player_.queueLast(track);

//     // ImGui::Separator();

//     // // Add to playlist submenu
//     // if (ImGui::BeginMenu("Add to Playlist")) {
//     //     if (player_.playlists().empty()) {
//     //         ImGui::TextDisabled("No playlists");
//     //     } else {
//     //         for (const Playlist& pl : player_.playlists()) {
//     //             if (ImGui::MenuItem(pl.getName().c_str()))
//     //                 player_.addTrackToPlaylist(pl.getName(), track);
//     //         }
//     //     }
//     //     ImGui::EndMenu();
//     // }

//     // ImGui::EndPopup();
// }