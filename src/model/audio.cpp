#include <thread>
#include <chrono>
#include <cstring>
#include <miniaudio.h>

#include "model/audio.hpp"

audio::audio(){
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format  = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate       = 48000;
    config.dataCallback     = audio::dataCallback;
    config.pUserData        = this;

    ma_device_init(NULL, &config, &device);
}

audio::~audio(){
    ma_device_uninit(&device);
    if(decoderInit)
        ma_decoder_uninit(&decoder);
}

void audio::dataCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount){
    audio* self = static_cast<audio*>(device->pUserData);
    if (!self->decoderInit || self->seeking) {
        // output silence during seek
        memset(output, 0, frameCount * 2 * sizeof(float));
        return;
    }
    
    ma_uint64 frameRead;
    ma_decoder_read_pcm_frames(&self->decoder, output, frameCount, &frameRead);

    if(frameRead < frameCount){
        size_t bytesRead    = frameRead * 2 * sizeof(float);
        size_t bytesTotal   = frameCount * 2 * sizeof(float);
        memset((char*)output + bytesRead, 0, bytesTotal - bytesRead);
        self->trackEnded = true;
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
    return trackEnded;
}

void audio::resetTrackEnded(){
    trackEnded = false;
}

void audio::load(const std::filesystem::path& musicpath){
    if(decoderInit){
        fadeOut();
        ma_device_stop(&device);
        ma_decoder_uninit(&decoder);
    }

    ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
    ma_decoder_init_file(musicpath.string().c_str(), &decoderConfig, &decoder);
    decoderInit = true;
    play();
}

void audio::play(){
    ma_device_set_master_volume(&device, 0.0f);
    ma_device_start(&device);
    fadeIn();
}

void audio::pause(){
    fadeOut();
    ma_device_stop(&device);
    ma_device_set_master_volume(&device, userVolume);
}

void audio::seek(float second){
    if (!decoderInit) return;
    seeking = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ma_uint64 frame = (ma_uint64)(second * decoder.outputSampleRate);
    ma_decoder_seek_to_pcm_frame(&decoder, frame);
    seeking = false;
}

void audio::setVolume(float volume){
    userVolume = volume;
    ma_device_set_master_volume(&device, userVolume);
}
const float audio::getVolume() const{
    return userVolume;
}