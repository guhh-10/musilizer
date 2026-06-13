cmake_minimum_required(VERSION 4.0)
project(musilizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# Dependencies (FetchContent)
# ==========================================

include(FetchContent)

FetchContent_Declare(
    IconFontCppHeader
    GIT_REPOSITORY  https://github.com/juliettef/IconFontCppHeaders.git
    GIT_TAG         210b5a399a64270674560d633638952d1e8d804d
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(
    tag
    GIT_REPOSITORY  https://github.com/taglib/taglib.git
    GIT_TAG         v2.3
    GIT_SHALLOW     TRUE
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY  https://github.com/mackron/miniaudio.git
    GIT_TAG         0.11.25
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(
    json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.12.0
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(
    Catch2 
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v3.5.4
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-3.4.10
    GIT_SHALLOW     TRUE
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY  https://github.com/ocornut/imgui.git
    GIT_TAG         v1.91.6
    GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(IconFontCppHeader tag miniaudio json Catch2 SDL3 imgui)

# TagLib include dirs
set(TAGLIB_INCLUDE_DIRS
    ${tag_BINARY_DIR}/taglib
    ${tag_SOURCE_DIR}/taglib
    ${tag_SOURCE_DIR}/taglib/toolkit
    ${tag_SOURCE_DIR}/taglib/mpeg
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v2
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v2/frames
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v1
)

# ImGui (no CMakeLists — compile sources directly)
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC SDL3::SDL3)

# ==========================================
# Warnings helper
# ==========================================

function(musilizer_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

# ==========================================
# Core library
# ==========================================

add_library(musilizer_core STATIC
    src/model/library.cpp
    src/model/music_directory.cpp
    src/model/audio.cpp
    src/model/audio_impl.cpp
    src/model/queue.cpp
    src/model/playlist.cpp
    src/model/play_history.cpp
    src/repository/persistence.cpp
    src/service/search.cpp
    src/controller/player.cpp
    src/controller/search_controller.cpp
    src/service/genre_graph.cpp
    src/service/recommender.cpp
    src/service/genre_graph_learner.cpp
)

target_include_directories(musilizer_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${miniaudio_SOURCE_DIR}
    ${TAGLIB_INCLUDE_DIRS}
)

target_link_libraries(musilizer_core PUBLIC
    tag
    nlohmann_json::nlohmann_json
)

musilizer_set_warnings(musilizer_core)

# ==========================================
# UI library
# ==========================================

add_library(musilizer_ui STATIC
    src/ui/main_window.cpp
    src/ui/player_panel.cpp
    src/ui/library_panel.cpp
    src/ui/playlist_panel.cpp
)

target_include_directories(musilizer_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${iconfontcppheader_SOURCE_DIR}
)

target_link_libraries(musilizer_ui PUBLIC
    musilizer_core
    imgui_lib
)

# ImGui uses a lot of old-style casts and other things that trip -Werror.
# We own musilizer_ui but not imgui itself, so set warnings only on our target.
musilizer_set_warnings(musilizer_ui)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(musilizer_ui PRIVATE -Wno-unused-private-field)
endif()

# ==========================================
# Main executable
# ==========================================

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE musilizer_ui)
target_include_directories(${PROJECT_NAME} PRIVATE ${iconfontcppheader_SOURCE_DIR})
musilizer_set_warnings(${PROJECT_NAME})

# Deploy lucide.ttf next to the executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts/lucide.ttf"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lucide.ttf"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts/InterVariable.ttf"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/InterVariable.ttf"
    COMMENT "Deploying fonts to build execution directory"
)

# ==========================================
# Test executable
# ==========================================

add_executable(musilizer_tests
    tests/test_audio.cpp
    tests/test_library.cpp
    tests/test_music_directory.cpp
    tests/test_persistence.cpp
    tests/test_play_history.cpp
    tests/test_playlist.cpp
    tests/test_queue.cpp
    tests/test_search.cpp
    tests/test_recommender.cpp
    tests/test_learner.cpp
)
target_link_libraries(musilizer_tests PRIVATE musilizer_core Catch2::Catch2WithMain)
musilizer_set_warnings(musilizer_tests)

# CTest / Catch2 test discovery
enable_testing()
include(CTest)
include(Catch)

catch_discover_tests(musilizer_tests
    TEST_SPEC "~[audio]"
    PROPERTIES LABELS unit
)
catch_discover_tests(musilizer_tests
    TEST_SPEC "[audio]"
    PROPERTIES LABELS audio
)

# ==========================================
# OS Deployment Configurations (Post-Build)
# ==========================================

# SDL3 DLL copy (Windows only)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying SDL3.dll"
    )
endif()