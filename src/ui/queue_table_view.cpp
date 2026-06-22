#include <imgui.h>
#include <imgui_internal.h>

#include <string>

#include "ui/fonts.hpp"
#include "ui/imgui_widgets.hpp"
#include "ui/queue_table_view.hpp"
#include "utils/time_format.hpp"

void RenderQueueTable(const std::vector<const Track*>& tracks, const Track* currentTrack, const QueueViewActions& actions)
{
    ImGuiTableFlags table_flags = ImGuiTableFlags_NoBordersInBody
                                | ImGuiTableFlags_NoHostExtendX;

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 7.0f));

    if (ImGui::BeginTable("##queue_table", 3, table_flags, ImVec2(0, 0)))
    {
        ImGui::TableSetupColumn("Number",   ImGuiTableColumnFlags_WidthFixed,   30.0f);
        ImGui::TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthStretch,  1.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed,   45.0f);

        static int selectedQueueIdx = -1;

        for (int i = 0; i < (int)tracks.size(); ++i)
        {
            const Track* t = tracks[i];
            ImGui::TableNextRow();

            ImGuiID row_id = ImGui::GetID((std::string("##queue_row_") + std::to_string(i)).c_str());
            float& t_hover = SmoothAnimState::GetRef(row_id);
            float animation_speed = 12.0f;

            ImGui::TableSetColumnIndex(1);
            float row_top_y = ImGui::GetCursorPosY();

            std::string title = t ? (t->getTitle().empty() ? t->getMusicPath().filename().string() : t->getTitle()) : "(unknown)";
            bool isCurrentTrack = (currentTrack && t && currentTrack->getMusicPath() == t->getMusicPath());
            bool isNextTrack = (i == 1 && tracks.size() > 1);

            ImGui::TextUnformatted(title.c_str());

            if (t && !t->getArtists().empty()) {
                ImGui::PushFont(FontManager::qArtist());
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);

                std::string artistStr;
                for (std::size_t a = 0; a < t->getArtists().size(); ++a) {
                    if (a) artistStr += ", ";
                    artistStr += t->getArtists()[a];
                }
                ImGui::TextUnformatted(artistStr.c_str());
                ImGui::PopStyleColor();
                ImGui::PopFont();
            }

            float row_bottom_y = ImGui::GetCursorPosY();
            float content_height = row_bottom_y - row_top_y;
            float single_line_height = ImGui::GetFontSize();
            float vertical_offset = (content_height - single_line_height) * 0.5f;

            ImGui::TableSetColumnIndex(0);

            ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(0,0,0,0));

            std::string selectable_id = "##queue_sel_" + std::to_string(i);
            bool is_selected = (selectedQueueIdx == i);

            ImGui::SetCursorPosY(row_top_y);
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

            bool item_clicked = ImGui::Selectable(selectable_id.c_str(), is_selected,
                                  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
                                  ImVec2(0, content_height));

            bool row_hovered = ImGui::IsItemHovered();
            ImGui::PopItemFlag();
            ImGui::PopStyleColor(3);

            if (item_clicked && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsPopupOpen("")) {
                selectedQueueIdx = i;
                if (actions.playIndex) {
                    actions.playIndex(static_cast<std::size_t>(i));
                }
            }

            if (ImGui::BeginPopupContextItem(selectable_id.c_str())) {
                selectedQueueIdx = i;

                const char* opt1 = " Move Up";
                const char* opt2 = " Move Down";
                const char* opt3 = " Delete from Queue";
                float auto_width = ImMax(ImGui::CalcTextSize(opt1).x,
                                    ImMax(ImGui::CalcTextSize(opt2).x, ImGui::CalcTextSize(opt3).x)) +
                                    (ImGui::GetStyle().ItemInnerSpacing.x * 2.0f);

                bool canMoveUp = (i > 0 && !isCurrentTrack && !isNextTrack);
                ImGui::BeginDisabled(!canMoveUp);
                if (ImGui::Selectable(opt1, false, 0, ImVec2(auto_width, 0))) {
                    if (canMoveUp && actions.moveUp) actions.moveUp(static_cast<std::size_t>(i));
                }
                if (!canMoveUp && i > 0) {
                    if (ImGui::IsItemHovered()) {
                        if (isCurrentTrack) {
                            ImGui::SetTooltip("Cannot move the currently playing track");
                        } else if (isNextTrack) {
                            ImGui::SetTooltip("Cannot move the next track up");
                        } else {
                            ImGui::SetTooltip("Cannot move the first track up");
                        }
                    }
                }
                ImGui::EndDisabled();

                bool canMoveDown = (i < (int)tracks.size() - 1 && !isCurrentTrack);
                ImGui::BeginDisabled(!canMoveDown);
                if (ImGui::Selectable(opt2, false, 0, ImVec2(auto_width, 0))) {
                    if (canMoveDown && actions.moveDown) actions.moveDown(static_cast<std::size_t>(i));
                }
                if (!canMoveDown && i < (int)tracks.size() - 1) {
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Cannot move the currently playing track down");
                    }
                }
                ImGui::EndDisabled();

                bool canDelete = !isCurrentTrack;
                ImGui::BeginDisabled(!canDelete);
                if (ImGui::Selectable(opt3, false, 0, ImVec2(auto_width, 0))) {
                    if (canDelete && actions.remove) actions.remove(static_cast<std::size_t>(i));
                }
                if (!canDelete) {
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Cannot delete the currently playing track");
                    }
                }
                ImGui::EndDisabled();

                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (vertical_offset > 0.0f) {
                ImGui::SetCursorPosY(row_top_y + vertical_offset);
            }

            std::string numStr = std::to_string(i + 1);
            float text_width = ImGui::CalcTextSize(numStr.c_str()).x;
            float column_width = ImGui::GetContentRegionAvail().x;
            if (text_width < column_width) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (column_width - text_width) * 0.5f);
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            ImGui::TextUnformatted(numStr.c_str());
            ImGui::PopStyleColor();

            ImGui::TableSetColumnIndex(2);
            if (vertical_offset > 0.0f) {
                ImGui::SetCursorPosY(row_top_y + vertical_offset);
            }

            std::string duration = t ? utils::formatDuration(t->getDuration()) : "";
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            ImGui::TextUnformatted(duration.c_str());
            ImGui::PopStyleColor();

            ImGui::SetCursorPosY(row_bottom_y);

            if (row_hovered) {
                t_hover += ImGui::GetIO().DeltaTime * animation_speed;
                if (t_hover > 1.0f) t_hover = 1.0f;
            } else {
                t_hover -= ImGui::GetIO().DeltaTime * animation_speed;
                if (t_hover < 0.0f) t_hover = 0.0f;
            }

            if (t_hover > 0.01f) {
                ImGuiTable* table = ImGui::GetCurrentTable();
                const float rounding = 6.0f;
                const float inset_x  = 4.0f;
                const float inset_y  = 2.0f;

                ImVec2 min_p = ImVec2(table->WorkRect.Min.x + inset_x, table->RowPosY1 + inset_y);
                ImVec2 max_p = ImVec2(table->WorkRect.Max.x - inset_x, table->RowPosY2 - inset_y);

                ImVec4 col_bg = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(col_bg.x, col_bg.y, col_bg.z, col_bg.w * t_hover));

                ImGui::TablePushBackgroundChannel();
                ImGui::GetCurrentWindow()->DrawList->AddRectFilled(min_p, max_p, row_bg_color, rounding);
                ImGui::TablePopBackgroundChannel();
            }
        }

        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
}
