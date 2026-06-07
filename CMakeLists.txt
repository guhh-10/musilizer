cmake_minimum_required(VERSION 4.3)
project(musilizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies

include(FetchContent)

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

FetchContent_MakeAvailable(tag miniaudio json Catch2)

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

# Warnings helper

function(musilizer_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

# Core library

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

# Main executable

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE musilizer_core)
musilizer_set_warnings(${PROJECT_NAME})

# Test executable

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