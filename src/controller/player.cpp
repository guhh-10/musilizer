#include <iostream>
#include <algorithm>

#include "controller/player.hpp"

Player::Player(Library& lib)
    : lib_(lib)
    , learner_(Recommender::buildDefaultGraph())
    , recommender_(learner_.toGraph())
{}

// ── emit helpers ──────────────────────────────────────────────────────────────

void Player::emitTrackChanged()         const { if (onTrackChanged)          onTrackChanged(nowPlaying_); }
void Player::emitPlaybackStateChanged() const { if (onPlaybackStateChanged)  onPlaybackStateChanged(playbackState_); }
void Player::emitVolumeChanged(float v) const { if (onVolumeChanged)         onVolumeChanged(v); }
void Player::emitQueueChanged()         const { if (onQueueChanged)          onQueueChanged(); }
void Player::emitRecommendationsReady() const { if (onRecommendationsReady)  onRecommendationsReady(); }
void Player::emitPlaylistsChanged()     const { if (onPlaylistsChanged)      onPlaylistsChanged(); }

// ── private helpers ───────────────────────────────────────────────────────────

bool Player::loadAudio(const fs::path& path) {
    const Track* track = lib_.findByPath(path);
    if (!track) {
        std::cerr << "[Player] track not found: " << path << "\n";
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
        std::cerr << "[Player] failed to load: " << e.what() << "\n";
        return false;
    }
}

void Player::loadTrack(const fs::path& path) {
    if (loadAudio(path))
        if (history_.current() != path)
            history_.push(*nowPlaying_);
}

void Player::observeTransition(const Track* prev, const Track* next, bool skipped) {
    if (!prev || !next) return;
    if (prev->getGenres().empty() || next->getGenres().empty()) return;
    if (skipped)
        learner_.observeSkip(prev->getGenres(), next->getGenres());
    else
        learner_.observeCompletion(prev->getGenres(), next->getGenres());
    recommender_.setGraph(learner_.toGraph());
    emitRecommendationsReady();
}

// ── playback ──────────────────────────────────────────────────────────────────

void Player::play(const Track& t) {
    queue_.load({ &t });
    emitQueueChanged();
    if (loadAudio(t.getMusicPath()))
        history_.push(t);
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
        std::cerr << "[Player] resume failed: " << e.what() << "\n";
    }
}

void Player::next() {
    const Track* prev = nowPlaying_;
    auto path = queue_.next();

    if (!path && prev) {
        auto recs = recommend(5);
        if (!recs.empty()) {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<std::size_t> dis(0, recs.size() - 1);
            const Track* sel = recs[dis(gen)].track;
            queue_.addTrackToBack(*sel);
            path = sel->getMusicPath();
        }
    }
    if (!path) return;
    loadTrack(*path);
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::previous() {
    const Track* prev = nowPlaying_;
    auto path = history_.back();
    if (!path) return;
    if (!loadAudio(*path)) return;
    queue_.load({ lib_.findByPath(*path) });
    emitQueueChanged();
    observeTransition(prev, nowPlaying_, /*skipped=*/true);
}

void Player::seek(float seconds) { audio_.seek(seconds); }

void Player::setVolume(float v) {
    audio_.setVolume(v);
    emitVolumeChanged(v);
}

// ── queue management ──────────────────────────────────────────────────────────

void Player::queueNext(const Track& t) { queue_.addTrackToFront(t); emitQueueChanged(); }
void Player::queueLast(const Track& t) { queue_.addTrackToBack(t);  emitQueueChanged(); }
void Player::setShuffle(bool e) { queue_.setShuffle(e); emitQueueChanged(); }
void Player::setRepeat(bool e)  { queue_.setRepeat(e);  emitQueueChanged(); }

std::vector<fs::path> Player::queueSnapshot() const {
    return queue_.snapshot();
}

std::vector<const Track*> Player::queueTracks() const {
    auto paths = queue_.snapshot();
    std::vector<const Track*> out;
    out.reserve(paths.size());
    for (const auto& p : paths)
        out.push_back(lib_.findByPath(p));
    return out;
}

// ── playlist management ───────────────────────────────────────────────────────

void Player::addPlaylist(Playlist p) {
    playlists_.push_back(std::move(p));
    emitPlaylistsChanged();
}

void Player::removePlaylist(const std::string& name) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it == playlists_.end()) return;

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
    emitTrackChanged();
    emitPlaybackStateChanged();
    emitPlaylistsChanged();
}

void Player::addTrackToPlaylist(const std::string& name, const Track& track) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it == playlists_.end()) return;
    it->addTrack(track);
    emitPlaylistsChanged();
}

void Player::removeTrackFromPlaylist(const std::string& name, const fs::path& path) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it == playlists_.end()) return;
    it->removeTrack(path);
    emitPlaylistsChanged();
}

void Player::moveTrackInPlaylist(const std::string& name, int from, int to) {
    auto it = std::find_if(playlists_.begin(), playlists_.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it == playlists_.end()) return;
    it->moveTrack(from, to);
    emitPlaylistsChanged();
}

const std::vector<Playlist>& Player::playlists() const { return playlists_; }

// ── persistence ───────────────────────────────────────────────────────────────

void Player::loadState() {
    float volume; bool shuffle, repeat;
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

// ── event loop ────────────────────────────────────────────────────────────────

void Player::update() {
    if (!audio_.hasTrackEnded()) return;
    const Track* prev = nowPlaying_;
    if (!prev) return;

    auto path = queue_.next();
    if (!path) {
        auto recs = recommend(5);
        if (!recs.empty()) {
            std::mt19937 gen(std::random_device{}());
            std::uniform_int_distribution<std::size_t> dis(0, recs.size() - 1);
            const Track* sel = recs[dis(gen)].track;
            queue_.addTrackToBack(*sel);
            path = sel->getMusicPath();
        }
    }
    if (!path) {
        audio_.resetTrackEnded();
        nowPlaying_    = nullptr;
        playbackState_ = PlaybackState::Stopped;
        emitTrackChanged();
        emitPlaybackStateChanged();
        return;
    }
    loadTrack(*path);
    observeTransition(prev, nowPlaying_, /*skipped=*/false);
}

// ── read state ────────────────────────────────────────────────────────────────

const Track*  Player::currentTrack()  const { return nowPlaying_; }
PlaybackState Player::playbackState() const { return playbackState_; }
float         Player::volume()        const { return audio_.getVolume(); }
bool          Player::isShuffle()     const { return queue_.isShuffle(); }
bool          Player::isRepeat()      const { return queue_.isRepeat(); }
float         Player::position()      const { return audio_.getPosition(); }

int Player::currentDuration() const {
    return nowPlaying_ ? nowPlaying_->getDuration() : 0;
}

std::vector<RecommendResult> Player::recommend(std::size_t limit) const {
    if (!nowPlaying_) return {};
    return recommender_.recommend(lib_, *nowPlaying_, limit);
}

void Player::playPlaylistStartingAt(const Playlist& p, int startIndex) {
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;

    if (startIndex < 0 || startIndex >= (int)tracks.size()) {
        startIndex = 0;
    }

    // Build the remaining loop sequence starting dynamically from the double-clicked position
    std::vector<const Track*> shiftedTracks;
    shiftedTracks.reserve(tracks.size());

    // 1. Queue clicked item through the end of the list
    for (int i = startIndex; i < (int)tracks.size(); ++i) {
        shiftedTracks.push_back(tracks[i]);
    }
    // 2. Wrap around and append the beginning items to the end of the playlist loop
    for (int i = 0; i < startIndex; ++i) {
        shiftedTracks.push_back(tracks[i]);
    }

    queue_.load(shiftedTracks);
    emitQueueChanged();

    if (!shiftedTracks.empty() && shiftedTracks[0]) {
        loadTrack(shiftedTracks[0]->getMusicPath());
    }
}

void Player::playQueueIndex(std::size_t index) {
    // Get the current track pointers list representing the queue items
    auto tracks = queueTracks();
    if (index >= tracks.size()) return;

    const Track* prev = nowPlaying_;

    // Slice the track vector to only include the target item and everything after it
    std::vector<const Track*> remainingTracks(tracks.begin() + index, tracks.end());

    // Re-initialize the queue container with this subset
    queue_.load(remainingTracks);
    emitQueueChanged();

    // Load and play the chosen track immediately
    if (!remainingTracks.empty() && remainingTracks[0]) {
        loadTrack(remainingTracks[0]->getMusicPath());
        observeTransition(prev, nowPlaying_, /*skipped=*/true);
    }
}