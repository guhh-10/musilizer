#include <iostream>

#include "controller/player.hpp"

Player::Player(Library& lib)
    : lib_(lib)
    , learner_(Recommender::buildDefaultGraph())   // seed prior from default graph
    , recommender_(learner_.toGraph())             // initial graph from prior only
{}

void Player::loadTrack(const fs::path& path) {
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[Player] track not found in library: " << path << "\n";
        return;
    }
    try {
        audio_.load(path);
        history_.push(*track);
        nowPlaying_   = track;
        trackSkipped_ = false;
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to load track: " << e.what() << "\n";
    }
}

void Player::observeTransition(const Track* prev,
                               const Track* next,
                               bool         skipped)
{
    if (!prev || !next) return;
    if (prev->getGenres().empty() || next->getGenres().empty()) return;
 
    if (skipped)
        learner_.observeSkip(prev->getGenres(), next->getGenres());
    else
        learner_.observeCompletion(prev->getGenres(), next->getGenres());
 
    recommender_.setGraph(learner_.toGraph());
}

void Player::play(const Track& t) {
    loadTrack(t.getMusicPath());
}

void Player::playPlaylist(const Playlist& p) {
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;
    queue_.load(tracks);
    loadTrack(queue_.current().value());
}

void Player::pause() {
    audio_.pause();
}

void Player::resume() {
    try {
        audio_.play();
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to resume: " << e.what() << "\n";
    }
}

void Player::next() {
    trackSkipped_ = true;
 
    const Track* prev = nowPlaying_;
    auto path = queue_.next();
    if (!path) return;
 
    loadTrack(*path);                       // sets nowPlaying_ to next track
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::previous() {
    trackSkipped_ = true;
 
    const Track* prev = nowPlaying_;
    auto path = history_.back();
    if (!path) return;
 
    audio_.load(*path);
    const Track* t = lib_.findByPath(*path);
    if (t) {
        nowPlaying_   = t;
        trackSkipped_ = false;
    }
    // previous() is navigating backward — we record it as a skip from
    // prev toward whatever track restores, but not as a forward transition.
    // Only record if both tracks are known.
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::seek(float seconds) {
    audio_.seek(seconds);
}

void Player::setVolume(float v) {
    audio_.setVolume(v);
}

void Player::queueNext(const Track& t) {
    queue_.addTrackToFront(t);
}

void Player::queueLast(const Track& t) {
    queue_.addTrackToBack(t);
}

void Player::setShuffle(bool enabled) {
    queue_.setShuffle(enabled);
}

void Player::setRepeat(bool enabled) {
    queue_.setRepeat(enabled);
}

void Player::loadState() {
    float volume;
    bool shuffle, repeat;
    Persistence::loadSettings(volume, shuffle, repeat);
    setVolume(volume);
    setShuffle(shuffle);
    setRepeat(repeat);
 
    playlists_ = Persistence::loadPlaylists(lib_);
    Persistence::loadHistory(history_, lib_);
    Persistence::loadLearner(learner_);
 
    // Rebuild graph after loading persisted counts.
    recommender_.setGraph(learner_.toGraph());
}

void Player::saveState() {
    Persistence::saveSettings(audio_.getVolume(), queue_.isShuffle(), queue_.isRepeat());
    Persistence::savePlaylists(playlists_);
    Persistence::saveHistory(history_);
    Persistence::saveLearner(learner_);
}

void Player::update() {
    if (!audio_.hasTrackEnded()) return;
 
    audio_.resetTrackEnded();
 
    // Natural completion — the track played to the end.
    const Track* prev = nowPlaying_;
    auto path = queue_.next();
    if (path) {
        loadTrack(*path);
        // nowPlaying_ is now the new track; prev is the completed track.
        observeTransition(prev, nowPlaying_, /*skipped=*/false);
    }
}

const Track* Player::currentTrack() const {
    auto path = queue_.current();
    if (!path) return nullptr;
    return lib_.findByPath(*path);
}

float Player::volume() const {
    return audio_.getVolume();
}

bool Player::isShuffle() const {
    return queue_.isShuffle();
}

bool Player::isRepeat() const {
    return queue_.isRepeat();
}

std::vector<RecommendResult> Player::recommend(std::size_t limit) const {
    const Track* current = currentTrack();
    if (!current) return {};
    return recommender_.recommend(lib_, *current, limit);
}