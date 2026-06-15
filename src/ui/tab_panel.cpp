#include "imgui.h"

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

        if (SmoothRadioButton("Playlists", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == 0))
            m_activeTab = 0;
            
        ImGui::SameLine(0, spacing);
        
        if (SmoothRadioButton("Up Next", {buttonW, buttonH}, ButtonFont::Bold, m_activeTab == 1))
            m_activeTab = 1;
            
    ImGui::PopStyleColor();

    ImGui::PopStyleVar(); // Restore global rounding and padding
}

void TabPanel::drawTabContent()
{
    // TODO: Implement conditional layout swap based on active tab state
}