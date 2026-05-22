#pragma once
#include <miniaudio.h>
#include <filesystem>

class audio{
    private:
        ma_device   device;
        ma_decoder  decoder;
        bool        decoderInit = false;

        static void dataCallback(
            ma_device* device, void* output, const void* input, ma_uint32 frameCount);

    public:
        audio();
        ~audio();

        void load(const std::filesystem::path& musicpath);
        void play();
        void pause();
        void seek(float second);
        // TODO:
        // void getPosition() const;
        void setVolume(float volume);
};