#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>
#include <miniaudio.h>

#include "model/audio.hpp"

Audio::Audio(){
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format  = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate       = 48000;
    config.dataCallback     = Audio::dataCallback;
    config.pUserData        = this;

    ma_result result = ma_device_init(NULL, &config, &device);
    if(result != MA_SUCCESS)
        throw std::runtime_error(
            std::string("Audio: failed to initialise device: ") +
            ma_result_description(result));
}

Audio::~Audio(){
    ma_device_stop(&device);
    {
        std::lock_guard<std::mutex> lock(decoder_mutex);
        if(decoder_initialized.load())
            ma_decoder_uninit(&decoder);
        decoder_initialized.store(false);
    }
    ma_device_uninit(&device);
}

void Audio::dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount){
    Audio* self = static_cast<Audio*>(device->pUserData);
    if (!self->decoder_initialized.load() || self->seeking.load()) {
        memset(output, 0, frameCount * 2 * sizeof(float));
        return;
    }

    std::unique_lock<std::mutex> lock(self->decoder_mutex, std::try_to_lock);
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
        self->track_ended.store(true);
    }

    (void)input;
}

void Audio::fadeOut(){
    float vol = user_volume.load();
    float step = vol / 10.0f;
    float volume = vol;
    for (int i = 0; i < 10; i++) {
        volume -= step;
        ma_device_set_master_volume(&device, std::max(volume, 0.0f));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Audio::fadeIn(){
    float target = user_volume.load();
    float step = target / 10.0f;
    float volume = 0.0f;
    for (int i = 0; i < 10; i++) {
        volume += step;
        ma_device_set_master_volume(&device, volume);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ma_device_set_master_volume(&device, target);
}

bool Audio::hasTrackEnded() const{ 
    return track_ended.load();
}

void Audio::resetTrackEnded(){
    track_ended.store(false);
}

void Audio::load(const fs::path& music_path){
    if(decoder_initialized.load()){
        fadeOut();
        ma_device_stop(&device);
    }

    {
        std::lock_guard<std::mutex> lock(decoder_mutex);
        if(decoder_initialized.load())
            ma_decoder_uninit(&decoder);

        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
        ma_result result = ma_decoder_init_file(
            music_path.string().c_str(), &decoderConfig, &decoder);
        if(result != MA_SUCCESS)
            throw std::runtime_error(
                std::string("Audio: failed to open '") +
                music_path.string() + "': " +
                ma_result_description(result));
        decoder_initialized.store(true);
    }

    play();
}

void Audio::play(){
    ma_device_set_master_volume(&device, 0.0f);
    ma_result result = ma_device_start(&device);
    if(result != MA_SUCCESS)
        throw std::runtime_error(
            std::string("Audio: failed to start device: ") +
            ma_result_description(result));
    fadeIn();
}

void Audio::pause(){
    fadeOut();
    ma_device_stop(&device);
    ma_device_set_master_volume(&device, user_volume.load());
}

void Audio::seek(float second){
    if (!decoder_initialized.load()) return;
    seeking.store(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    {
        std::lock_guard<std::mutex> lock(decoder_mutex);
        ma_uint64 frame = (ma_uint64)(second * decoder.outputSampleRate);
        ma_result result = ma_decoder_seek_to_pcm_frame(&decoder, frame);
        if(result != MA_SUCCESS)
            std::cerr << "[Audio::seek] failed: " << ma_result_description(result) << "\n";
    }

    seeking.store(false);
}

float Audio::getPosition() const{
    if (!decoder_initialized.load()) return 0.0f;
    std::lock_guard<std::mutex> lock(decoder_mutex);
    ma_uint64 cursor = 0;
    ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursor);
    return static_cast<float>(cursor) / decoder.outputSampleRate;
}

void Audio::setVolume(float volume){
    user_volume.store(volume);
    ma_device_set_master_volume(&device, volume);
}

float Audio::getVolume() const{
    return user_volume.load();
}