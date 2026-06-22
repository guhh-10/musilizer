#include "controller/playback_coordinator.hpp"
#include <iostream>
#include <random>

PlaybackCoordinator::PlaybackCoordinator(Library& lib, RecommendationCoordinator& recCoord)
    : lib_(lib)
    , recommendationCoordinator_(recCoord)
{}

void PlaybackCoordinator::emitTrackChanged()         const { if (onTrackChanged)         onTrackChanged(nowPlaying_); }
void PlaybackCoordinator::emitPlaybackStateChanged() const { if (onPlaybackStateChanged) onPlaybackStateChanged(playbackState_); }
void PlaybackCoordinator::emitVolumeChanged(float v) const { if (onVolumeChanged)        onVolumeChanged(v); }
void PlaybackCoordinator::emitQueueChanged()         const { if (onQueueChanged)         onQueueChanged(); }

bool PlaybackCoordinator::loadAudio(const fs::path& path) {
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[PlaybackCoordinator] track not found: " << path << "\n";
        return false;
    }
    try {
        audio_.load(path);
        nowPlaying_    = track;
        playbackState_ = PlaybackState::Playing;
        emitTrackChanged();
        emitPlaybackStateChanged();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[PlaybackCoordinator] failed to load: " << e.what() << "\n";
        return false;
    }
}

void PlaybackCoordinator::loadTrack(const fs::path& path) {
    if (loadAudio(path)) {
        if (history_.current() != path) {
            history_.push(*nowPlaying_);
        }
    }
}

void PlaybackCoordinator::play(const Track& t) {
    queue_.load({ &t });
    emitQueueChanged();
    if (loadAudio(t.getMusicPath())) {
        history_.push(t);
    }
}

void PlaybackCoordinator::pause() {
    audio_.pause();
    playbackState_ = PlaybackState::Paused;
    emitPlaybackStateChanged();
}

void PlaybackCoordinator::resume() {
    try {
        audio_.play();
        playbackState_ = PlaybackState::Playing;
        emitPlaybackStateChanged();
    } catch (const std::exception& e) {
        std::cerr << "[PlaybackCoordinator] resume failed: " << e.what() << "\n";
    }
}

void PlaybackCoordinator::stop() {
    audio_.pause();
    nowPlaying_ = nullptr;
    playbackState_ = PlaybackState::Stopped;
    emitTrackChanged();
    emitPlaybackStateChanged();
}

std::optional<fs::path> PlaybackCoordinator::pickAndEnqueueRecommendation(const Track& prev) {
    auto recs = recommendationCoordinator_.recommend(lib_, prev, 5);
    if (recs.empty()) return std::nullopt;

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dis(0, recs.size() - 1);
    const Track* sel = recs[dis(gen)].track;
    queue_.addTrackToBack(*sel);
    return sel->getMusicPath();
}

void PlaybackCoordinator::next() {
    const Track* prev = nowPlaying_;
    auto path = queue_.next();

    if (!path && prev) {
        path = pickAndEnqueueRecommendation(*prev);
    }
    if (!path) return;
    loadTrack(*path);
    recommendationCoordinator_.observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void PlaybackCoordinator::previous() {
    const Track* prev = nowPlaying_;
    auto path = history_.back();
    if (!path) return;
    if (!loadAudio(*path)) return;
    queue_.load({ lib_.findByPath(*path) });
    emitQueueChanged();
    recommendationCoordinator_.observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void PlaybackCoordinator::seek(float seconds) { audio_.seek(seconds); }

void PlaybackCoordinator::setVolume(float v) {
    audio_.setVolume(v);
    emitVolumeChanged(v);
}

void PlaybackCoordinator::queueNext(const Track& t) { queue_.addTrackToFront(t); emitQueueChanged(); }
void PlaybackCoordinator::queueLast(const Track& t) { queue_.addTrackToBack(t);  emitQueueChanged(); }
void PlaybackCoordinator::setShuffle(bool e) { queue_.setShuffle(e); emitQueueChanged(); }
void PlaybackCoordinator::setRepeat(bool e)  { queue_.setRepeat(e);  emitQueueChanged(); }

void PlaybackCoordinator::playQueueIndex(std::size_t index) {
    auto tracks = queueTracks();
    if (index >= tracks.size()) return;

    const Track* prev = nowPlaying_;
    std::vector<const Track*> remainingTracks(tracks.begin() + index, tracks.end());

    queue_.load(remainingTracks);
    emitQueueChanged();

    if (!remainingTracks.empty() && remainingTracks[0]) {
        loadTrack(remainingTracks[0]->getMusicPath());
        recommendationCoordinator_.observeTransition(prev, nowPlaying_, /*skipped=*/true);
    }
}

void PlaybackCoordinator::moveQueueTrackUp(std::size_t index) {
    auto tracks = queueTracks();
    if (index == 0 || index >= tracks.size()) return;
    std::swap(tracks[index], tracks[index - 1]);
    queue_.load(tracks);
    emitQueueChanged();
}

void PlaybackCoordinator::moveQueueTrackDown(std::size_t index) {
    auto tracks = queueTracks();
    if (index + 1 >= tracks.size()) return;
    std::swap(tracks[index], tracks[index + 1]);
    queue_.load(tracks);
    emitQueueChanged();
}

void PlaybackCoordinator::removeQueueTrack(std::size_t index) {
    auto tracks = queueTracks();
    if (index >= tracks.size()) return;
    
    bool isCurrent = (index == 0);
    tracks.erase(tracks.begin() + index);
    queue_.load(tracks);
    emitQueueChanged();
    
    if (isCurrent) {
        if (tracks.empty()) {
            stop();
        } else {
            loadTrack(tracks[0]->getMusicPath());
        }
    }
}

void PlaybackCoordinator::loadQueue(const std::vector<const Track*>& tracks) {
    queue_.load(tracks);
    emitQueueChanged();
}

void PlaybackCoordinator::clearQueue() {
    queue_.load({});
    emitQueueChanged();
}

std::vector<fs::path> PlaybackCoordinator::queueSnapshot() const { return queue_.snapshot(); }

std::vector<const Track*> PlaybackCoordinator::queueTracks() const {
    auto paths = queue_.snapshot();
    std::vector<const Track*> out;
    out.reserve(paths.size());
    for (const auto& p : paths) {
        out.push_back(lib_.findByPath(p));
    }
    return out;
}

const Track* PlaybackCoordinator::currentTrack() const { return nowPlaying_; }
PlaybackState PlaybackCoordinator::playbackState() const { return playbackState_; }
float PlaybackCoordinator::volume() const { return audio_.getVolume(); }
bool PlaybackCoordinator::isShuffle() const { return queue_.isShuffle(); }
bool PlaybackCoordinator::isRepeat() const { return queue_.isRepeat(); }
float PlaybackCoordinator::position() const { return audio_.getPosition(); }
int PlaybackCoordinator::currentDuration() const { return nowPlaying_ ? nowPlaying_->getDuration() : 0; }

void PlaybackCoordinator::update() {
    if (!audio_.hasTrackEnded()) return;
    const Track* prev = nowPlaying_;
    if (!prev) return;

    auto path = queue_.next();
    if (!path) {
        path = pickAndEnqueueRecommendation(*prev);
    }
    if (!path) {
        audio_.resetTrackEnded();
        stop();
        return;
    }
    loadTrack(*path);
    recommendationCoordinator_.observeTransition(prev, nowPlaying_, /*skipped=*/false);
}

// For persistence
void PlaybackCoordinator::setVolumeInternal(float v) { audio_.setVolume(v); }
void PlaybackCoordinator::setShuffleInternal(bool enabled) { queue_.setShuffle(enabled); }
void PlaybackCoordinator::setRepeatInternal(bool enabled) { queue_.setRepeat(enabled); }