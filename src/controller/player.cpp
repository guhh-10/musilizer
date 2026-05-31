#include <iostream>

#include "controller/player.hpp"

void Player::loadTrack(const fs::path& path){
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[Player] track not found in library: " << path << "\n";
        return;
    }
    try {
        audio_.load(path);
        history_.push(*track);
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to load track: " << e.what() << "\n";
    }
}

Player::Player(Library& lib) : lib_(lib){}

void Player::play(const Track& t){
    loadTrack(t.getMusicPath());
}

void Player::playPlaylist(const Playlist& p){
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;
    queue_.load(tracks);
    loadTrack(queue_.current().value());
}

void Player::pause(){
    audio_.pause();
}

void Player::resume(){
    try {
        audio_.play();
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to resume: " << e.what() << "\n";
    }
}

void Player::next(){
    auto path = queue_.next();
    if (path) loadTrack(*path);
}

void Player::previous(){
    auto path = history_.back();
    if (path) audio_.load(*path);
}

void Player::seek(float seconds){
    audio_.seek(seconds);
}

void Player::setVolume(float v){
    audio_.setVolume(v);
}

void Player::queueNext(const Track& t){
    queue_.addTrackToFront(t);
}

void Player::queueLast(const Track& t){
    queue_.addTrackToBack(t);
}

void Player::setShuffle(bool enabled){
    queue_.setShuffle(enabled);
}

void Player::setRepeat(bool enabled){
    queue_.setRepeat(enabled);
}

void Player::loadState(){
    float volume;
    bool shuffle, repeat;
    Persistence::loadSettings(volume, shuffle, repeat);
    setVolume(volume);
    setShuffle(shuffle);
    setRepeat(repeat);

    playlists_ = Persistence::loadPlaylists(lib_);
    Persistence::loadHistory(history_, lib_);
}

void Player::saveState(){
    Persistence::saveSettings(audio_.getVolume(), queue_.isShuffle(), queue_.isRepeat());
    Persistence::savePlaylists(playlists_);
    Persistence::saveHistory(history_);
}

void Player::update(){
    if (audio_.hasTrackEnded()) {
        audio_.resetTrackEnded();
        next();
    }
}

const Track* Player::currentTrack() const{
    auto path = queue_.current();
    if (!path) return nullptr;
    return lib_.findByPath(*path);
}

float Player::volume() const{
    return audio_.getVolume();
}

bool Player::isShuffle() const{
    return queue_.isShuffle();
}

bool Player::isRepeat() const{
    return queue_.isRepeat();
}