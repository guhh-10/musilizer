#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <miniaudio.h>

#include "model/audio.hpp"

audio::audio(){
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format  = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate       = 48000;
    config.dataCallback     = audio::dataCallback;
    config.pUserData        = this;

    ma_result result = ma_device_init(NULL, &config, &device);
    if(result != MA_SUCCESS)
        throw std::runtime_error(
            std::string("audio: failed to initialise device: ") +
            ma_result_description(result));
}

audio::~audio(){
    ma_device_uninit(&device);
    if(decoderInit.load())
        ma_decoder_uninit(&decoder);
}

void audio::dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount){
    audio* self = static_cast<audio*>(device->pUserData);
    if (!self->decoderInit.load() || self->seeking.load()) {
        memset(output, 0, frameCount * 2 * sizeof(float));
        return;
    }

    std::unique_lock<std::mutex> lock(self->decoderMutex, std::try_to_lock);
    if (!lock.owns_lock()) {
        memset(output, 0, frameCount * 2 * sizeof(float));
        return;
    }
    
    ma_uint64 frameRead;
    ma_decoder_read_pcm_frames(&self->decoder, output, frameCount, &frameRead);

    if(frameRead < frameCount){
        size_t bytesRead    = frameRead * 2 * sizeof(float);
        size_t bytesTotal   = frameCount * 2 * sizeof(float);
        memset((char*)output + bytesRead, 0, bytesTotal - bytesRead);
        self->trackEnded.store(true);
    }

    (void)input;
}

void audio::fadeOut(){
    float step = userVolume / 10.0f;
    float volume = userVolume;
    for (int i = 0; i < 10; i++) {
        volume -= step;
        ma_device_set_master_volume(&device, volume);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void audio::fadeIn(){
    float target = userVolume;
    float step = userVolume / 10.0f;
    float volume = 0.0f;
    for (int i = 0; i < 10; i++) {
        volume += step;
        ma_device_set_master_volume(&device, volume);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ma_device_set_master_volume(&device, target); // make sure it lands exactly at 1.0
}

bool audio::hasTrackEnded() const{ 
    return trackEnded.load();
}

void audio::resetTrackEnded(){
    trackEnded.store(false);
}

void audio::load(const std::filesystem::path& musicpath){
    if(decoderInit.load()){
        fadeOut();
        ma_device_stop(&device);
    }

    {
        std::lock_guard<std::mutex> lock(decoderMutex);
        if(decoderInit.load())
            ma_decoder_uninit(&decoder);

        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
        ma_result result = ma_decoder_init_file(
            musicpath.string().c_str(), &decoderConfig, &decoder);
        if(result != MA_SUCCESS)
            throw std::runtime_error(
                std::string("audio: failed to open '") +
                musicpath.string() + "': " +
                ma_result_description(result));
        decoderInit.store(true);
    }

    play();
}

void audio::play(){
    ma_device_set_master_volume(&device, 0.0f);
    ma_result result = ma_device_start(&device);
    if(result != MA_SUCCESS)
        throw std::runtime_error(
            std::string("audio: failed to start device: ") +
            ma_result_description(result));
    fadeIn();
}

void audio::pause(){
    fadeOut();
    ma_device_stop(&device);
    ma_device_set_master_volume(&device, userVolume);
}

void audio::seek(float second){
    if (!decoderInit.load()) return;
    seeking.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    {
        std::lock_guard<std::mutex> lock(decoderMutex);
        ma_uint64 frame = (ma_uint64)(second * decoder.outputSampleRate);
        ma_result result = ma_decoder_seek_to_pcm_frame(&decoder, frame);
        if(result != MA_SUCCESS)
            std::cerr << "[audio::seek] failed: " << ma_result_description(result) << "\n";
    }

    seeking.store(false);
}

void audio::setVolume(float volume){
    userVolume = volume;
    ma_device_set_master_volume(&device, userVolume);
}
float audio::getVolume() const{
    return userVolume;
}