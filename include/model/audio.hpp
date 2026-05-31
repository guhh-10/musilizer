#pragma once
#include <miniaudio.h>
#include <atomic>
#include <mutex>

#include "config.hpp"

class Audio{
    private:
        ma_device   device;
        mutable ma_decoder decoder;
        mutable std::mutex decoder_mutex;
        std::atomic<float> user_volume{1.0f};
        std::atomic<bool>  seeking = false;
        std::atomic<bool>  track_ended = false;
        std::atomic<bool>  decoder_initialized = false;

        static void dataCallback(
            ma_device* device, void* output, const void* input, ma_uint32 frameCount);
        void fadeOut();
        void fadeIn();

    public:
        Audio();
        ~Audio();

        Audio(const Audio&)            = delete;
        Audio& operator=(const Audio&) = delete;
        Audio(Audio&&)                 = delete;
        Audio& operator=(Audio&&)      = delete;

        bool hasTrackEnded() const;
        void resetTrackEnded();
        void load(const fs::path& music_path);
        void play();
        void pause();
        void seek(float second);
        float getPosition() const;
        void setVolume(float volume);
        float getVolume() const;
};