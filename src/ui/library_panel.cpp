#include <imgui.h>
#include <imgui_internal.h>
#include <IconsLucide.h>
#include <vector>
#include <string>
#include <cstdio>

#include "controller/library_sort_state.hpp"
#include "ui/library_panel.hpp"
#include "ui/fonts.hpp"
#include "ui/imgui_widgets.hpp"
#include "utils/time_format.hpp"

// ── Column index constants ────────────────────────────────────────────────────
// Col 0 = "#" (row number – not sortable)
// Col 1 = Title
// Col 2 = Artist
// Col 3 = Duration
static constexpr int COL_TITLE    = 1;
static constexpr int COL_ARTIST   = 2;
static constexpr int COL_DURATION = 3;

// ── LibraryPanel ──────────────────────────────────────────────────────────────

LibraryPanel::LibraryPanel(Player& player, SearchController& search)
    : player_(player), search_(search)
{
    runSearch();
}

void LibraryPanel::runSearch() {
    SearchQuery q;
    q.text         = searchBuf_;
    q.artistFilter = artistFilter_;
    sortState_.applyTo(q);
    results_ = search_.query(q);
}

void LibraryPanel::setArtistFilter(const std::string& artist) {
    artistFilter_ = artist;
    runSearch();
}

// ── Column header helper ──────────────────────────────────────────────────────

// Draws a clickable, hover-animated column header cell.
// colIdx  – which column index this is (used to detect active sort)
// label   – visible text
// rightPad – shift label left by this much (used for "#" to right-align)
// indent  – shift label right by this much (used for data columns)
void LibraryPanel::drawColumnHeader(int colIdx, const char* label, float rightPad, float indent) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g    = *GImGui;
    ImGuiStyle& style  = g.Style;
    ImDrawList* dl     = window->DrawList;

    // Build the full header cell rect
    float colW  = ImGui::GetContentRegionAvail().x;
    float cellH = ImGui::GetTextLineHeight() + 16.0f; // same as data row approx
    ImVec2 pos  = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + colW, pos.y + cellH));

    const bool sortable = (colIdx != 0); // "#" column is not sortable
    ImGuiID id = sortable ? window->GetID(label) : 0;

    ImGui::ItemSize(bb);
    if (sortable && !ImGui::ItemAdd(bb, id)) return;
    if (!sortable) { ImGui::ItemAdd(bb, 0); }

    bool hovered = false, held = false, pressed = false;
    if (sortable) {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed) {
            sortState_.cycleColumn(colIdx);
            runSearch();
        }
    }

    // ── Hover background ──────────────────────────────────────────────────────
    float& t = SmoothAnimState::GetRef(id + 7777); // offset to avoid clash
    float speed = 12.0f;
    if (hovered || (sortState_.sortColumn() == colIdx && sortState_.sortState() != SortCycleState::NEUTRAL)) {
        t += g.IO.DeltaTime * speed; if (t > 1.0f) t = 1.0f;
    } else {
        t -= g.IO.DeltaTime * speed; if (t < 0.0f) t = 0.0f;
    }

    if (t > 0.01f) {
        ImVec4 base  = style.Colors[ImGuiCol_HeaderHovered];
        ImU32  bgCol = ImGui::GetColorU32(ImVec4(base.x, base.y, base.z, base.w * t * 0.6f));
        dl->AddRectFilled(bb.Min, bb.Max, bgCol, 0.0f);
    }

    // ── Label ─────────────────────────────────────────────────────────────────
    bool isActivelySorted = sortable && sortState_.sortColumn() == colIdx && sortState_.sortState() != SortCycleState::NEUTRAL;

    ImVec4 textCol = isActivelySorted
        ? style.Colors[ImGuiCol_Text]
        : style.Colors[ImGuiCol_TextDisabled];
    // Brighten text on hover even when not sorted
    if (hovered && !isActivelySorted)
        textCol = ImVec4(textCol.x * 1.3f, textCol.y * 1.3f, textCol.z * 1.3f, textCol.w);

    float centerY = (bb.Min.y + bb.Max.y) * 0.5f;
    ImVec2 labelSz = ImGui::CalcTextSize(label);

    if (colIdx == 0) {
        // "#" — right-aligned
        float x = bb.Max.x - rightPad - labelSz.x;
        dl->AddText(ImVec2(x, centerY - labelSz.y * 0.5f),
                    ImGui::GetColorU32(textCol), label);
    } else {
        float x = bb.Min.x + indent;
        dl->AddText(ImVec2(x, centerY - labelSz.y * 0.5f),
                    ImGui::GetColorU32(textCol), label);

        // ── Sort indicator ────────────────────────────────────────────────────
        if (sortable) {
            const char* indicator = nullptr;
            if (sortState_.sortColumn() == colIdx) {
                if (sortState_.sortState() == SortCycleState::ASC)  indicator = ICON_LC_CHEVRON_UP;
                if (sortState_.sortState() == SortCycleState::DESC) indicator = ICON_LC_CHEVRON_DOWN;
            }

            if (indicator) {
                // Add a manual 4px offset to prevent the icon from hugging the text
                float ix = x + labelSz.x + 4.0f;
                ImVec4 accentCol = style.Colors[ImGuiCol_SliderGrabActive];
                ImGui::PushFont(FontManager::icons());
                ImVec2 iSz = ImGui::CalcTextSize(indicator);
                dl->AddText(FontManager::icons(), FontManager::icons()->FontSize,
                            ImVec2(ix, centerY - iSz.y * 0.5f),
                            ImGui::GetColorU32(accentCol), indicator);
                ImGui::PopFont();
            } else if (hovered) {
                // Ghost up-arrow hint on hover when column has no current sort
                ImGui::PushFont(FontManager::icons());
                const char* ghost = ICON_LC_CHEVRON_UP;
                ImVec2 gSz = ImGui::CalcTextSize(ghost);
                ImVec4 ghostCol = style.Colors[ImGuiCol_TextDisabled];
                ghostCol.w *= t * 0.7f;
                dl->AddText(FontManager::icons(), FontManager::icons()->FontSize,
                            ImVec2(x + labelSz.x + 4.0f, centerY - gSz.y * 0.5f),
                            ImGui::GetColorU32(ghostCol), ghost);
                ImGui::PopFont();
            }
        }
    }

    // ── Cursor sort pointer ───────────────────────────────────────────────────
    if (sortable && hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

// ── Search bar ────────────────────────────────────────────────────────────────

void LibraryPanel::drawSearchBar(float padding) {
    ImGuiStyle&  style  = ImGui::GetStyle();
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    // Header row
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    ImGui::PushFont(FontManager::npTitle());
    ImGui::TextUnformatted("Library");
    ImGui::PopFont();
    ImGui::SameLine();

    // "Artist: <name>" indicator when filtered
    if (!artistFilter_.empty()) {
        ImGui::PushFont(FontManager::npArtist());
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_SliderGrabActive]);
        std::string label = "Artist: " + artistFilter_ + "  \xc3\x97"; // × symbol
        float tw = ImGui::CalcTextSize(label.c_str()).x;
        ImGui::SetCursorPosX(
            std::max(ImGui::GetCursorPosX(),
                     ImGui::GetWindowContentRegionMax().x - tw - padding));
        if (ImGui::SmallButton(label.c_str())) {
            artistFilter_.clear();
            runSearch();
        }
        ImGui::PopStyleColor();
        ImGui::PopFont();
    } else {
        // Track count right-aligned
        ImGui::PushFont(FontManager::npArtist());
        char countBuf[32];
        snprintf(countBuf, sizeof(countBuf), "%d tracks", (int)results_.size());
        float tw = ImGui::CalcTextSize(countBuf).x;
        float target = std::max(ImGui::GetCursorPosX(),
                                ImGui::GetWindowContentRegionMax().x - tw - padding);
        ImGui::SetCursorPosX(target);
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted(countBuf);
        ImGui::PopStyleColor();
        ImGui::PopFont();
    }

    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    // Search input panel
    const ImGuiID id = window->GetID("##library_search");
    float inputBarH  = ImGui::GetTextLineHeight() + style.FramePadding.y * 2.0f + 12.0f;
    float width_arg  = ImGui::GetWindowContentRegionMax().x
                       - ImGui::GetWindowContentRegionMin().x
                       - padding * 2.0f;
    const float px = 8.0f, py = 6.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
    ImVec2 pos = window->DC.CursorPos;
    ImRect panel_bb(pos, ImVec2(pos.x + width_arg, pos.y + inputBarH));

    ImGui::ItemSize(panel_bb, style.FramePadding.y);
    if (ImGui::ItemAdd(panel_bb, id)) {
        ImU32 bgCol     = IM_COL32(255, 255, 255, 255);
        ImU32 borderCol = ImGui::GetColorU32(style.Colors[ImGuiCol_Border]);
        window->DrawList->AddRectFilled(panel_bb.Min, panel_bb.Max, bgCol, 6.0f);
        window->DrawList->AddRect(panel_bb.Min, panel_bb.Max, borderCol, 6.0f, 0, 1.5f);

        // Search icon
        ImFont* iconFont  = FontManager::icons();
        ImVec2  iconSize  = iconFont->CalcTextSizeA(iconFont->FontSize, FLT_MAX, 0.0f, ICON_LC_SEARCH);
        float   centerY   = panel_bb.Min.y + inputBarH * 0.5f;
        window->DrawList->AddText(iconFont, iconFont->FontSize,
            ImVec2(panel_bb.Min.x + px, centerY - iconSize.y * 0.5f),
            ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]),
            ICON_LC_SEARCH);

        float iconW = iconSize.x + 6.0f;
        float inputW = width_arg - px * 2.0f - iconW;
        window->DC.CursorPos = ImVec2(panel_bb.Min.x + px + iconW, panel_bb.Min.y + py);

        ImGui::PushStyleColor(ImGuiCol_FrameBg,  style.Colors[ImGuiCol_Text]);
        ImGui::PushStyleColor(ImGuiCol_Text,     style.Colors[ImGuiCol_FrameBgActive]);
        if (SmoothActiveInputText("##library_search", searchBuf_, sizeof(searchBuf_),
                                  ImVec2(inputW, 0.0f)))
            runSearch();

        if (searchBuf_[0] == '\0') {
            ImVec2 hintPos = {
                panel_bb.Min.x + px + iconW + style.FramePadding.x,
                panel_bb.Min.y + py + style.FramePadding.y
            };
            window->DrawList->AddText(hintPos,
                ImGui::GetColorU32(style.Colors[ImGuiCol_TextDisabled]),
                "Search tracks, artists...");
        }
        ImGui::PopStyleColor(2);
    }
}

// ── Track table ───────────────────────────────────────────────────────────────

void LibraryPanel::drawTableTrack() {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoPadOuterX;
    const float rightPad          = 6.0f;
    const float contentLeftIndent = 12.0f;

    if (!ImGui::BeginTable("##library_table", 4, tableFlags, ImVec2(0.0f, 0.0f))) {
        ImGui::PopStyleVar();
        return;
    }

    ImGui::TableSetupColumn("#",        ImGuiTableColumnFlags_WidthFixed,   28.0f);
    ImGui::TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthStretch, 50.0f);
    ImGui::TableSetupColumn("Artist",   ImGuiTableColumnFlags_WidthStretch, 35.0f);
    ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthStretch, 15.0f);

    // ── Header row ────────────────────────────────────────────────────────────
    // We draw headers manually (no TableHeadersRow) so we can attach click
    // behaviour and custom sort indicators to each cell.

    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImGui::GetStyle().Colors[ImGuiCol_Border]);
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

    ImGui::TableSetColumnIndex(0);
    drawColumnHeader(0, "#", rightPad, 0.0f);

    ImGui::TableSetColumnIndex(1);
    drawColumnHeader(COL_TITLE, "Title", 0.0f, contentLeftIndent);

    ImGui::TableSetColumnIndex(2);
    drawColumnHeader(COL_ARTIST, "Artist", 0.0f, contentLeftIndent);

    ImGui::TableSetColumnIndex(3);
    drawColumnHeader(COL_DURATION, "Duration", 0.0f, contentLeftIndent);

    ImGui::PopStyleColor(); // TableHeaderBg

    // ── Data rows ─────────────────────────────────────────────────────────────
    const float rowH         = ImGui::GetTextLineHeight() + 16.0f;
    const Track* activeTrack = player_.currentTrack();

    for (int i = 0; i < (int)results_.size(); ++i) {
        const Track* track = results_[i].track;
        if (!track) continue;

        ImGui::TableNextRow(ImGuiTableRowFlags_None, rowH);
        bool isActive = (activeTrack == track);

        ImGui::PushStyleColor(ImGuiCol_Header,        IM_COL32(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  IM_COL32(0,0,0,0));

        ImGui::TableSetColumnIndex(0);
        ImVec2 cellPos   = ImGui::GetCursorScreenPos();
        float  colW      = ImGui::GetContentRegionAvail().x;
        float  lineH     = ImGui::GetTextLineHeight();
        float  offsetY   = (rowH - lineH) * 0.5f;
        float  centerY   = cellPos.y + offsetY + lineH * 0.5f;
        float  rightEdge = cellPos.x + colW - rightPad;

        std::string selId  = "##row_" + std::to_string(i);
        std::string ctxId  = "##ctx_" + std::to_string(i);

        if (ImGui::Selectable(selId.c_str(), isActive, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, rowH))) {
            player_.play(*track);
        }
        bool rowHov = ImGui::IsItemHovered();
        ImGui::PopStyleColor(3);

        // Background highlighting
        if (isActive || rowHov) {
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,1));
            ImU32 bgCol = isActive ? ImGui::GetColorU32(ImGuiCol_HeaderActive) : ImGui::GetColorU32(ImGuiCol_HeaderHovered);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, bgCol);
            ImGui::PopStyleColor();
        }

        // Context Menu Setup
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(8.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);
        if (ImGui::BeginPopupContextItem(ctxId.c_str())) {
            drawContextMenu(track);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(3);

        // Column 0 — Number / play icon
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (rowHov || isActive) {
            ImFont* iFont   = FontManager::icons();
            ImVec2  iSize   = iFont->CalcTextSizeA(iFont->FontSize, FLT_MAX, 0.0f, ICON_LC_PLAY);
            ImU32   iCol    = isActive ? ImGui::GetColorU32(ImGuiCol_PlotLinesHovered) : ImGui::GetColorU32(ImGuiCol_Text);
            dl->AddText(iFont, iFont->FontSize, ImVec2(rightEdge - iSize.x, centerY - iSize.y * 0.5f), iCol, ICON_LC_PLAY);
        } else {
            char numBuf[12];
            snprintf(numBuf, sizeof(numBuf), "%d", i + 1);
            ImVec2 ts = ImGui::CalcTextSize(numBuf);
            dl->AddText(ImVec2(rightEdge - ts.x, centerY - ts.y * 0.5f), ImGui::GetColorU32(ImGuiCol_TextDisabled), numBuf);
        }

        // Column 1: Title
        ImGui::TableSetColumnIndex(1);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentLeftIndent);
        if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_SeparatorActive]);
        const std::string& title = track->getTitle().empty() ? track->getMusicPath().filename().string() : track->getTitle();
        ImGui::TextUnformatted(title.c_str());
        if (isActive) ImGui::PopStyleColor();

        // Column 2: Artist list
        ImGui::TableSetColumnIndex(2);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentLeftIndent);
        std::string artistStr;
        for (std::size_t a = 0; a < track->getArtists().size(); ++a) {
            if (a) artistStr += ", ";
            artistStr += track->getArtists()[a];
        }
        if (!isActive && !rowHov) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted(artistStr.c_str());
        if (!isActive && !rowHov) ImGui::PopStyleColor();

        // Column 3: Duration
        ImGui::TableSetColumnIndex(3);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + contentLeftIndent);
        if (!isActive && !rowHov) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::TextUnformatted(utils::formatDuration(track->getDuration()).c_str());
        if (!isActive && !rowHov) ImGui::PopStyleColor();
    }

    ImGui::EndTable();
    ImGui::PopStyleVar();
}

// ── Context menu ──────────────────────────────────────────────────────────────

void LibraryPanel::drawContextMenu(const Track* track) {
    if (!track) return;

    if (ImGui::MenuItem("Play"))
        player_.play(*track);

    if (ImGui::MenuItem("Queue Next"))
        player_.queueNext(*track);

    if (ImGui::MenuItem("Queue Last"))
        player_.queueLast(*track);

    ImGui::Separator();

    if (ImGui::BeginMenu("Add to Playlist")) {
        // New playlist input
        static char newPlBuf[64] = "";
        ImGui::BeginGroup();
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 4.0f));
            ImGui::Indent(ImGui::GetStyle().FramePadding.x);

            if (SmoothActiveInputText("##ctx_newpl", newPlBuf, sizeof(newPlBuf),
                                      ImVec2(120.0f, 0.0f))) {}

            ImGui::SameLine();
            if (SmoothScaleButton(ICON_LC_PLUS, ImVec2(24.0f, 0.0f), ButtonFont::Icons)) {
                if (newPlBuf[0] != '\0') {
                    player_.addPlaylist(Playlist(newPlBuf));
                    player_.addTrackToPlaylist(newPlBuf, *track);
                    newPlBuf[0] = '\0';
                }
            }
            ImGui::Unindent(ImGui::GetStyle().FramePadding.x);
            ImGui::PopStyleVar();
        }
        ImGui::EndGroup();

        const auto& playlists = player_.playlists();
        if (!playlists.empty()) {
            ImGui::Separator();
            for (const Playlist& pl : playlists) {
                if (ImGui::MenuItem(pl.getName().c_str()))
                    player_.addTrackToPlaylist(pl.getName(), *track);
            }
        } else {
            ImGui::TextDisabled("No playlists");
        }

        ImGui::EndMenu();
    }
}