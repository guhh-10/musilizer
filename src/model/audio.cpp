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
    if (!self->decoderInit) return;
    ma_decoder_read_pcm_frames(&self->decoder, output, frameCount, nullptr);
    (void)input;
}

void audio::load(const std::filesystem::path& musicpath){
    ma_device_stop(&device);
    if(decoderInit)
        ma_decoder_uninit(&decoder);

    ma_decoder_init_file(musicpath.string().c_str(), NULL, &decoder);
    decoderInit = true;
    ma_device_start(&device);
}

void audio::play(){
    ma_device_start(&device);
}

void audio::pause(){
    ma_device_stop(&device);
}

void audio::seek(float second){
    if (!decoderInit) return;
    ma_uint64 frame = (ma_uint64)(second * decoder.outputSampleRate);
    ma_decoder_seek_to_pcm_frame(&decoder, frame);
}

void audio::setVolume(float volume){
    ma_device_set_master_volume(&device, volume);
}