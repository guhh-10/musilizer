cmake_minimum_required(VERSION 4.3)
project(musilizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    tag
    GIT_REPOSITORY  https://github.com/taglib/taglib.git
    GIT_TAG         v2.3
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tag)

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY  https://github.com/mackron/miniaudio.git
    GIT_TAG         0.11.25
)
FetchContent_MakeAvailable(miniaudio)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/model/library.cpp
    src/model/musicDirectory.cpp
    src/model/audio.cpp
    src/model/audio_impl.cpp
    src/model/queue.cpp
    src/model/playlist.cpp
    src/model/playHistory.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include

    # Miniaudio include directory
    ${miniaudio_SOURCE_DIR}

    # TagLib core
    ${tag_BINARY_DIR}/taglib
    ${tag_SOURCE_DIR}/taglib
    ${tag_SOURCE_DIR}/taglib/toolkit

    # ID3v2 (MP3 tags)
    ${tag_SOURCE_DIR}/taglib/mpeg
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v2
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v2/frames

    # ID3v1 (legacy, might be needed alongside v2)
    ${tag_SOURCE_DIR}/taglib/mpeg/id3v1
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    tag
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()