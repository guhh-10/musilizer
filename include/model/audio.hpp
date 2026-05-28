#pragma once
#include <miniaudio.h>
#include <filesystem>
#include <atomic>

class audio{
    private:
        ma_device   device;
        ma_decoder  decoder;
        float       userVolume = 1.0f;
        bool        decoderInit = false;
        std::atomic<bool> seeking = false;
        std::atomic<bool> trackEnded = false;

        static void dataCallback(
            ma_device* device, void* output, const void* input, ma_uint32 frameCount);
        void fadeOut();
        void fadeIn();

    public:
        audio();
        ~audio();

        bool hasTrackEnded() const;
        void resetTrackEnded();
        void load(const std::filesystem::path& musicpath);
        void play();
        void pause();
        void seek(float second);
        // TODO:
        // void getPosition() const;
        void setVolume(float volume);
};