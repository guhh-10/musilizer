#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <IconsLucide.h>

#include <iostream>

#include "config.hpp"
#include "model/library.hpp"
#include "model/music_directory.hpp"
#include "controller/player.hpp"
#include "controller/search_controller.hpp"
#include "repository/persistence.hpp"
#include "ui/main_window.hpp"

static void InitFonts(const fs::path& exeDir) {
    ImGuiIO& io = ImGui::GetIO();

    fs::path uiFont = exeDir / "InterVariable.ttf";
    io.Fonts->AddFontFromFileTTF(uiFont.string().c_str(), 16.0f);

    static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode  = true;
    icons_config.PixelSnapH = true;

    fs::path iconFont = exeDir / "lucide.ttf";
    io.Fonts->AddFontFromFileTTF(iconFont.string().c_str(), 13.0f, &icons_config, icons_ranges);
}

int main([[maybe_unused]] int argc, char* argv[]) {
    // ── Backend init ──────────────────────────────────────────────────────────

    fs::path exeDir = fs::weakly_canonical(argv[0]).parent_path();
    fs::path projectRootDir = exeDir.parent_path();
    config::init(projectRootDir);
    Persistence::init();

    Library lib;
    MusicDirectory().initialize(lib);

    Player player(lib);
    SearchController search(lib);

    // ── SDL3 init ─────────────────────────────────────────────────────────────
    // SDL_INIT_AUDIO is intentionally omitted — miniaudio owns the audio device.

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[SDL] Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Musilizer", 1440, 680,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (!window) {
        std::cerr << "[SDL] CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "[SDL] CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetWindowMinimumSize(window, 1440, 680);

    // ── ImGui init ────────────────────────────────────────────────────────────

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    InitFonts(exeDir);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // ── UI + state ────────────────────────────────────────────────────────────

    MainWindow ui(player, search);
    player.loadState();

    // ── Main loop ─────────────────────────────────────────────────────────────

    bool running = true;
    while (running) {
        // Event pump
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // Advance playback state (track-end detection, queue advancement)
        player.update();

        // Build UI
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ui.draw();

        // Render
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // ── Teardown ──────────────────────────────────────────────────────────────

    player.saveState();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}