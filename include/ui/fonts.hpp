#pragma once
#include <imgui.h>
#include <string>

class FontManager {
public:
    static void init(const std::string& exeDir);

    // Core fonts
    static ImFont* regular()       { return s_regular; }
    static ImFont* bold()          { return s_bold; }
    static ImFont* icons()         { return s_icons; }
    static ImFont* largeIcons()    { return s_large_icons; }

    // Pre-baked UI Roles
    static ImFont* npTitle()       { return s_np_title; }      // Now Playing: 16px Bold
    static ImFont* npArtist()      { return s_np_artist; }     // Now Playing: 13px Regular

    static ImFont* qTitle()        { return s_q_title; }       // Queue: 14px Bold
    static ImFont* qArtist()       { return s_q_artist; }      // Queue: 12px Regular

    static ImFont* tTitleArtist()  { return s_t_title_artist; } // Library: 14px Regular

private:
    static inline ImFont* s_regular        = nullptr;
    static inline ImFont* s_bold           = nullptr;
    static inline ImFont* s_icons          = nullptr;
    static inline ImFont* s_large_icons    = nullptr;

    static inline ImFont* s_np_title       = nullptr;
    static inline ImFont* s_np_artist      = nullptr;
    static inline ImFont* s_q_title        = nullptr;
    static inline ImFont* s_q_artist       = nullptr;
    static inline ImFont* s_t_title_artist = nullptr;
};