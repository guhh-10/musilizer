#include <iostream>
#include <algorithm>

#include "controller/player.hpp"

Player::Player(Library& lib)
    : lib_(lib)
    , learner_(Recommender::buildDefaultGraph())
    , recommender_(learner_.toGraph())
{}

// emit helpers

void Player::emitTrackChanged() const { 
    if (onTrackChanged) onTrackChanged(nowPlaying_); 
}

void Player::emitPlaybackStateChanged() const { 
    if (onPlaybackStateChanged) onPlaybackStateChanged(playbackState_); 
}

void Player::emitVolumeChanged(float v) const { 
    if (onVolumeChanged) onVolumeChanged(v); 
}

void Player::emitQueueChanged() const { 
    if (onQueueChanged) onQueueChanged(); 
}

void Player::emitRecommendationsReady() const { 
    if (onRecommendationsReady) onRecommendationsReady(); 
}

void Player::emitPlaylistsChanged() const { 
    if (onPlaylistsChanged) onPlaylistsChanged(); 
}

// private helpers

bool Player::loadAudio(const fs::path& path) {
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[Player] track not found in library: " << path << "\n";
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
        std::cerr << "[Player] failed to load track: " << e.what() << "\n";
        return false;
    }
}

void Player::loadTrack(const fs::path& path) {
    if (loadAudio(path)) {
        // Only push to history if it's not the exact same track we are already looking at
        if (history_.current() != path) {
            history_.push(*nowPlaying_);
        }
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
    emitRecommendationsReady();
}

// playback

void Player::play(const Track& t) {
    // Prime the manual queue wrapper to cleanly hold this track context
    queue_.load({ &t });
    emitQueueChanged();
    
    if (loadAudio(t.getMusicPath())) {
        history_.push(t);
    }
}

void Player::playPlaylist(const Playlist& p) {
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;
    queue_.load(tracks);
    loadTrack(queue_.current().value());
    emitQueueChanged();
}

void Player::pause() {
    audio_.pause();
    playbackState_ = PlaybackState::Paused;
    emitPlaybackStateChanged();
}

void Player::resume() {
    try {
        audio_.play();
        playbackState_ = PlaybackState::Playing;
        emitPlaybackStateChanged();
    } catch (const std::exception& e) {
        std::cerr << "[Player] failed to resume: " << e.what() << "\n";
    }
}

void Player::next() {
    const Track* prev = nowPlaying_;
    auto path = queue_.next();
    
    // If user's explicit queue runs out, request pool from Recommender
    if (!path && prev) {
        auto recommendations = recommend(5);
        if (!recommendations.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<std::size_t> dis(0, recommendations.size() - 1);
            
            const Track* randomSelection = recommendations[dis(gen)].track;
            queue_.addTrackToBack(*randomSelection);
            path = randomSelection->getMusicPath();
        }
    }

    if (!path) return;
    loadTrack(*path);
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::previous() {
    const Track* prev = nowPlaying_;

    // Fetch the structural historical item behind our current track
    auto path = history_.back();
    if (!path) return;

    if (!loadAudio(*path)) return;

    // Synchronize queue visual tracking state to point to the historical item if available
    queue_.load({ lib_.findByPath(*path) });
    emitQueueChanged();

    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::seek(float seconds) {
    audio_.seek(seconds);
}

void Player::setVolume(float v) {
    audio_.setVolume(v);
    emitVolumeChanged(v);
}

// queue management

void Player::queueNext(const Track& t) {
    queue_.addTrackToFront(t);
    emitQueueChanged();
}

void Player::queueLast(const Track& t) {
    queue_.addTrackToBack(t);
    emitQueueChanged();
}

void Player::setShuffle(bool enabled) {
    queue_.setShuffle(enabled);
    emitQueueChanged();
}

void Player::setRepeat(bool enabled) {
    queue_.setRepeat(enabled);
    emitQueueChanged();
}

// playlist management

void Player::addPlaylist(Playlist p) {
    playlists_.push_back(std::move(p));
    emitPlaylistsChanged();
}

void Player::removePlaylist(const std::string& name) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it != playlists_.end()) {
        if (nowPlaying_) {
            const auto& paths = it->getPlaylistTracks();
            bool inDying = std::find(paths.begin(), paths.end(),
                               nowPlaying_->getMusicPath()) != paths.end();
            if (inDying) {
                audio_.pause();
                nowPlaying_    = nullptr;
                playbackState_ = PlaybackState::Stopped;
                queue_.load({});
            }
        }
        playlists_.erase(it);
        emitTrackChanged();       // ← tell UI nowPlaying_ is null BEFORE
        emitPlaybackStateChanged();
        emitPlaylistsChanged();   // ← then update playlist panel
    }
}

void Player::addTrackToPlaylist(const std::string& playlistName, const Track& track) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == playlistName; });
    if (it == playlists_.end()) return;
    it->addTrack(track);
    emitPlaylistsChanged();
}

const std::vector<Playlist>& Player::playlists() const {
    return playlists_;
}

// persistence

void Player::loadState() {
    float volume;
    bool shuffle, repeat;
    Persistence::loadSettings(volume, shuffle, repeat);

    audio_.setVolume(volume);
    queue_.setShuffle(shuffle);
    queue_.setRepeat(repeat);

    playlists_ = Persistence::loadPlaylists(lib_);
    Persistence::loadHistory(history_, lib_);
    Persistence::loadLearner(learner_);
    recommender_.setGraph(learner_.toGraph());

    emitVolumeChanged(volume);
    emitQueueChanged();
    emitPlaylistsChanged();
}

void Player::saveState() {
    Persistence::saveSettings(audio_.getVolume(), queue_.isShuffle(), queue_.isRepeat());
    Persistence::savePlaylists(playlists_);
    Persistence::saveHistory(history_);
    Persistence::saveLearner(learner_);
}

// event loop

void Player::update() {
    // 1. Check if the song naturally finished
    if (!audio_.hasTrackEnded()) return;

    // REMOVED: Do not reset the audio flag here! Let Audio::load() handle it.

    const Track* prev = nowPlaying_;
    if (!prev) return;

    // 3. Advance the queue securely
    auto path = queue_.next();
    
    // 4. Recommender Auto-Advance fallback logic if queue runs out completely
    if (!path) {
        auto recommendations = recommend(5);
        if (!recommendations.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<std::size_t> dis(0, recommendations.size() - 1);
            
            const Track* randomSelection = recommendations[dis(gen)].track;
            queue_.addTrackToBack(*randomSelection);
            path = randomSelection->getMusicPath();
        }
    }

    // 5. If there is absolutely nothing left to play, stop the engine cleanly
    if (!path) {
        audio_.resetTrackEnded(); // ADDED: Clear the flag here so we don't infinitely loop
        nowPlaying_    = nullptr;
        playbackState_ = PlaybackState::Stopped;
        emitTrackChanged();
        emitPlaybackStateChanged();
        return;
    }

    // 6. Load the validated track path
    loadTrack(*path);
    observeTransition(prev, nowPlaying_, /*skipped=*/false);
}

// read state

const Track* Player::currentTrack() const {
    return nowPlaying_;
}

PlaybackState Player::playbackState() const {
    return playbackState_;
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

float Player::position() const {
    return audio_.getPosition();
}

std::vector<RecommendResult> Player::recommend(std::size_t limit) const {
    if (!nowPlaying_) return {};
    return recommender_.recommend(lib_, *nowPlaying_, limit);
}