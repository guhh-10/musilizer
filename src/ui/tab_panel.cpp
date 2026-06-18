#include "imgui.h"
#include <imgui_internal.h>
#include <vector>
#include <string>

#include "ui/tab_panel.hpp"
#include "ui/imgui_widgets.hpp"

void TabPanel::drawTabBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    const ImGuiStyle& style = ImGui::GetStyle();
    const float spacing = style.ItemSpacing.x;
    
    float totalWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
    const float buttonW = (totalWidth - spacing) / 2.0f; 
    
    // Use the window's inner height instead of the available region to avoid animated resize feedback loops
    const float buttonH = ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));

        if (SmoothRadioButton("Playlists", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == ContextView::VIEW_PLAYLIST))
            m_activeTab = ContextView::VIEW_PLAYLIST;
            
        ImGui::SameLine(0, spacing);
        
        if (SmoothRadioButton("Up Next", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == ContextView::VIEW_QUEUE))
            m_activeTab = ContextView::VIEW_QUEUE;
            
    ImGui::PopStyleColor();

    ImGui::PopStyleVar(); // Restore global rounding and padding
}

void TabPanel::drawTabContent()
{
    if (m_activeTab == ContextView::VIEW_PLAYLIST)
        drawPlaylistContent();
    else if (m_activeTab == ContextView::VIEW_QUEUE)
        drawQueueContent();
}

void TabPanel::drawPlaylistContent()
{
    static const std::vector<PlaylistGroup> mock_playlists = {
        {
            "Synthwave Essentials", 
            {
                {"Laser Driver", "4:12"},
                {"Outrun Horizon", "3:58"},
                {"Grid Crawler", "5:04"}
            }
        },
        {
            "Chill Ambient Vibes", 
            {
                {"Liquid Ether", "6:21"},
                {"Dust Particles", "2:45"}
            }
        },
        {
            "Empty Collection Focus", 
            {}
        }
    };

    StrictTwoTierPlaylistView("##playlist_tree_view", mock_playlists);
}

void TabPanel::drawQueueContent()
{
    static const std::vector<TableRowItem> mock_queue = {
        {"1",  "Starlight Express",   "The Midnight",      "3:44"},
        {"2",  "Midnight Coffee Run", "Timecop1983",      "3:05"},
        {"3",  "Cybernetic Dreams",   "FM-84",             "5:12"},
        {"4",  "Whispering Shadows",  "Gunship",           "4:34"},
        {"5",  "Solar Wind Flares",   "Scandroid",         "3:19"},
        {"6",  "Neon Drift",          "Kavinsky",          "4:02"},
        {"7",  "Glass Horizon",       "Michael McCann",    "3:51"},
        {"8",  "Velvet Circuit",      "Waveshaper",        "2:58"},
    };

    static int selected_queue_idx = -1;

    SmoothHoverTable("##queue_table", mock_queue, &selected_queue_idx);
}