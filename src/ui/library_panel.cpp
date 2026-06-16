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

void LibraryPanel::drawSearchBar(float padding) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    // ------------------------------------------------------------------------
    // INTERNAL STEP 1: RENDER THE HEADER (Inside Child A)
    // ------------------------------------------------------------------------
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    ImGui::PushFont(FontManager::npTitle());
    ImGui::Text("Library");
    ImGui::PopFont();
    ImGui::SameLine();

    ImGui::PushFont(FontManager::npArtist());
    float right_text_width = ImGui::CalcTextSize("Track Number").x; 
    ImGui::PopFont();

    // Calculate alignment bound to this child's max width limit
    float target_pos_x = ImGui::GetWindowContentRegionMax().x - right_text_width - padding;
    if (target_pos_x < ImGui::GetCursorPosX()) {
        target_pos_x = ImGui::GetCursorPosX(); 
    }
    ImGui::SetCursorPosX(target_pos_x);

    ImGui::PushFont(FontManager::npArtist());
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    ImGui::Text("Track Number");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    // Spacing between text headers and the search input loop
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    // ------------------------------------------------------------------------
    // INTERNAL STEP 2: RENDER THE SEARCH BAR INPUT
    // ------------------------------------------------------------------------
    const ImGuiID id = window->GetID("##library_search");
    
    // Explicit bounding height strictly for the input bar graphics block
    float input_bar_graphic_height = ImGui::GetTextLineHeight() + (style.FramePadding.y * 2.0f) + (6.0f * 2.0f);
    float width_arg = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - (padding * 2.0f);
    float padding_x = 8.0f;
    float padding_y = 6.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    ImVec2 pos = window->DC.CursorPos;
    ImRect panel_bb(pos, ImVec2(pos.x + width_arg, pos.y + input_bar_graphic_height));

    ImGui::ItemSize(panel_bb, style.FramePadding.y);
    if (ImGui::ItemAdd(panel_bb, id))
    {
        // Explicit solid white background panel (Overrides the child container background)
        ImU32 white_bg_col = IM_COL32(255, 255, 255, 255); 
        ImU32 darker_white_border_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Border]);
        float panel_rounding = 6.0f;

        window->DrawList->AddRectFilled(panel_bb.Min, panel_bb.Max, white_bg_col, panel_rounding);
        window->DrawList->AddRect(panel_bb.Min, panel_bb.Max, darker_white_border_col, panel_rounding, 0, 1.5f);

        // Procedural magnifying glass icon
        float center_y = panel_bb.Min.y + (input_bar_graphic_height * 0.5f);
        ImVec2 icon_center = ImVec2(panel_bb.Min.x + padding_x + 6.0f, center_y - 2.0f);
        float icon_radius = 4.5f;
        ImU32 icon_color = ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]);

        window->DrawList->AddCircle(icon_center, icon_radius, icon_color, 16, 2.0f);
        ImVec2 handle_start = ImVec2(icon_center.x + 3.0f, icon_center.y + 3.0f);
        ImVec2 handle_end = ImVec2(icon_center.x + 8.0f, icon_center.y + 8.0f);
        window->DrawList->AddLine(handle_start, handle_end, icon_color, 2.5f);

        // Embedded text input
        float icon_allocated_width = 24.0f;
        float embedded_input_width = width_arg - (padding_x * 2.0f) - icon_allocated_width;
        window->DC.CursorPos = ImVec2(panel_bb.Min.x + padding_x + icon_allocated_width, panel_bb.Min.y + padding_y);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Text]);
        ImGui::PushStyleColor(ImGuiCol_Text,    style.Colors[ImGuiCol_FrameBgActive]);   

        if (SmoothActiveInputText("##library_search", searchBuf_, sizeof(searchBuf_), ImVec2(embedded_input_width, 0)))
            runSearch();

        if (searchBuf_[0] == '\0') {
            ImVec2 hint_pos = ImVec2(
                panel_bb.Min.x + padding_x + icon_allocated_width + style.FramePadding.x,
                panel_bb.Min.y + padding_y + style.FramePadding.y
            );
            window->DrawList->AddText(hint_pos, ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]), "Search tracks, artists...");
        }

        ImGui::PopStyleColor(2);
    }
}

void LibraryPanel::drawTableTrack() {
    ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoPadOuterX;
    const float right_padding = 6.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(16.0f, 8.0f));
    // Using 0.0f, 0.0f auto-stretches the table completely to fill the host child wrapper bounds
    if (ImGui::BeginTable("##static_library_table", 4, table_flags, ImVec2(0.0f, 0.0f)))
    {
        // Inside library_panel.cpp -> LibraryPanel::drawTableTrack()

        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 28.0f);
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch, 50.0f);
        ImGui::TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthStretch, 35.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch, 15.0f);

        // 1. Push the header text styles AND the new table header background color
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImGui::GetStyle().Colors[ImGuiCol_Border]); // <-- PUSH BORDER COLOR

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

        // Column 0 Header Alignment
        ImGui::TableSetColumnIndex(0);
        {
            ImVec2 cell_pos = ImGui::GetCursorScreenPos();
            float column_width = ImGui::GetContentRegionAvail().x;
            ImVec2 text_size = ImGui::CalcTextSize("#");
            float right_edge_x = cell_pos.x + column_width - right_padding;
            ImGui::GetWindowDrawList()->AddText(ImVec2(right_edge_x - text_size.x, cell_pos.y), ImGui::GetColorU32(ImGuiCol_Text), "#");
            ImGui::Dummy(ImVec2(0, text_size.y)); 
        }

        ImGui::TableSetColumnIndex(1); ImGui::TableHeader("Title");
        ImGui::TableSetColumnIndex(2); ImGui::TableHeader("Artist");
        ImGui::TableSetColumnIndex(3); ImGui::TableHeader("Duration");

        // 2. Pop 4 colors instead of 3 to clean up the stack properly
        ImGui::PopStyleColor(4); // <-- CHANGED FROM 3 TO 4
        
        static const std::vector<StaticTrack> static_tracks = {
            {"Starlight Express", {"Neon Horizon", "Lumina"}, 224, "starlight_express.mp4"},
            {"Midnight Coffee Run", {"The Caffeine Code"}, 185, "midnight_coffee.wav"},
            {"Cybernetic Dreams", {"Glitch Overlord"}, 312, "cyber dreams.mp3"},
            {"Whispering Shadows", {"Echo Location", "Acoustic Mirage"}, 274, "whispering_shadows.flac"},
            {"Solar Wind Flares", {"Cosmic Radiation"}, 199, "solar_flares.ogg"}
        };

        static int selected_mock_idx = -1;
        int row_number = 0;

        for (const auto& track : static_tracks) {
            row_number++;
            ImGui::TableNextRow();

            bool is_active = (selected_mock_idx == row_number);

            // Push transparent colors so the Selectable itself draws no highlight
            ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(0,0,0,0));

            ImGui::TableSetColumnIndex(0);

            ImVec2 cell_pos = ImGui::GetCursorScreenPos();
            float column_width = ImGui::GetContentRegionAvail().x;
            float line_height = ImGui::GetTextLineHeight();

            std::string row_select_id  = "##static_row_"    + std::to_string(row_number);
            std::string row_context_id = "##row_ctx_menu_"  + std::to_string(row_number);

            if (ImGui::Selectable(row_select_id.c_str(), is_active, ImGuiSelectableFlags_SpanAllColumns)) {
                selected_mock_idx = row_number;
            }

            bool row_hovered = ImGui::IsItemHovered();

            ImGui::PopStyleColor(3);   // pop the three transparent colors

            // Apply the row background colour using table API (fills entire row including padding)
            if (is_active || row_hovered) {
                ImU32 bg_color = is_active ? ImGui::GetColorU32(ImGuiCol_HeaderActive)
                                           : ImGui::GetColorU32(ImGuiCol_HeaderHovered);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bg_color);
            }

            // Style vars must be pushed BEFORE BeginPopupContextItem so ImGui
            // applies them when it sizes and positions the popup window itself.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(8.0f, 6.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);

            // Each row must own its popup under a unique ID so right-clicking row N
            // cannot open the context window that was last opened on row M.
            if (ImGui::BeginPopupContextItem(row_context_id.c_str())) {
                selected_mock_idx = row_number;
                drawContextMenu(track.title, track.filename);
                ImGui::EndPopup();
            }

            ImGui::PopStyleVar(3);

            // Draw the right-aligned content in column 0 (play icon or row number)
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float right_edge_x = cell_pos.x + column_width - right_padding;
            float center_y = cell_pos.y + line_height * 0.5f;

            if (row_hovered || is_active) {
                const char* play_icon = ICON_LC_PLAY;
                ImFont* icon_font     = FontManager::icons();

                // Measure using the icon font itself, not the currently-pushed Inter font
                ImVec2 icon_size = icon_font->CalcTextSizeA(
                    icon_font->FontSize, FLT_MAX, 0.0f, play_icon);

                ImU32 icon_color = is_active
                    ? ImGui::GetColorU32(ImGuiCol_SliderGrabActive)
                    : ImGui::GetColorU32(ImGuiCol_Text);

                // Pass icon_font + its size explicitly; never relies on the ImGui font stack
                draw_list->AddText(
                    icon_font,
                    icon_font->FontSize,
                    ImVec2(right_edge_x - icon_size.x, center_y - icon_size.y * 0.5f),
                    icon_color,
                    play_icon
                );
            } else {
                char number_buf[8];
                snprintf(number_buf, sizeof(number_buf), "%d", row_number);
                ImVec2 text_size = ImGui::CalcTextSize(number_buf);
                draw_list->AddText(ImVec2(right_edge_x - text_size.x, center_y - text_size.y * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), number_buf);
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(track.title.c_str());

            ImGui::TableSetColumnIndex(2);
            std::string artist_str;
            for (size_t i = 0; i < track.artists.size(); ++i) {
                if (i > 0) artist_str += ", ";
                artist_str += track.artists[i];
            }
            ImGui::TextUnformatted(artist_str.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d:%02d", track.duration / 60, track.duration % 60);
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}

// Updated signature implementation at the bottom of library_panel.cpp
void LibraryPanel::drawContextMenu(const std::string& title, const std::string& filename) {
    // Silence unused parameter errors for the mock implementation
    (void)title;
    (void)filename;

    if (ImGui::MenuItem("Play")) {
        // player_.play(filename); 
    }

    if (ImGui::MenuItem("Queue Next")) {
        // player_.queueNext(filename);
    }

    if (ImGui::MenuItem("Queue Last")) {
        // player_.queueLast(filename);
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Add to Playlist")) {
        static const std::vector<std::string> mock_playlists = { "Favorites", "Chill Beats", "Driving Mix" };
        
        if (mock_playlists.empty()) {
            ImGui::TextDisabled("No playlists");
        } else {
            for (const auto& playlist_name : mock_playlists) {
                if (ImGui::MenuItem(playlist_name.c_str())) {
                    // player_.addTrackToPlaylist(playlist_name, filename);
                }
            }
        }
        ImGui::EndMenu();
    }
}