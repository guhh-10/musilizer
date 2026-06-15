#pragma once
#include <imgui.h>
#include <string>

class FontManager {
public:
    static void init(const std::string& exeDir);
    static ImFont* regular() { return s_regular; }
    static ImFont* bold()    { return s_bold; }
    static ImFont* icons()   { return s_icons; }
    static ImFont* largeIcons() { return s_large_icons; }
    
private:
    static inline ImFont* s_regular = nullptr;
    static inline ImFont* s_bold    = nullptr;
    static inline ImFont* s_icons   = nullptr;
    static inline ImFont* s_large_icons = nullptr;
};