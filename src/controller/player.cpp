#include <iostream>

#include "controller/player.hpp"

Player::Player(Library& lib)
    : lib_(lib)
    , learner_(Recommender::buildDefaultGraph())
    , recommender_(learner_.toGraph())
{}

bool Player::loadAudio(const fs::path& path) {
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[Player] track not found in library: " << path << "\n";
        return false;
    }
    try {
        audio_.load(path);
        nowPlaying_ = track;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to load track: " << e.what() << "\n";
        return false;
    }
}

void Player::loadTrack(const fs::path& path) {
    if (loadAudio(path))
        history_.push(*nowPlaying_);
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
    const Track* prev = nowPlaying_;

    auto path = queue_.next();
    if (!path) return;

    loadTrack(*path);
    // loadTrack sets nowPlaying_; observe the skip transition.
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::previous() {
    const Track* prev = nowPlaying_;

    auto path = history_.back();
    if (!path) return;

    if (!loadAudio(*path)) return;

    queue_.load({});

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

    const Track* prev = nowPlaying_;
    if (!prev) return;   // nothing was playing; nothing to advance from

    auto path = queue_.next();
    if (!path) return;

    loadTrack(*path);
    observeTransition(prev, nowPlaying_, /*skipped=*/false);
}

const Track* Player::currentTrack() const {
    return nowPlaying_;
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
    if (!nowPlaying_) return {};
    return recommender_.recommend(lib_, *nowPlaying_, limit);
}