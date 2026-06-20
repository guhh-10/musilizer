#include "controller/player.hpp"
#include "repository/session_persistence.hpp"
#include <algorithm>

Player::Player(Library& lib)
    : lib_(lib)
    , playlistStore_()
    , recommendationCoordinator_()
    , playbackCoordinator_(lib, recommendationCoordinator_)
{
    // Wire up callbacks
    playlistStore_.onPlaylistsChanged = [this]() {
        if (onPlaylistsChanged) onPlaylistsChanged();
    };
    recommendationCoordinator_.onRecommendationsReady = [this]() {
        if (onRecommendationsReady) onRecommendationsReady();
    };
    playbackCoordinator_.onTrackChanged = [this](const Track* t) {
        if (onTrackChanged) onTrackChanged(t);
    };
    playbackCoordinator_.onPlaybackStateChanged = [this](PlaybackState s) {
        if (onPlaybackStateChanged) onPlaybackStateChanged(s);
    };
    playbackCoordinator_.onVolumeChanged = [this](float v) {
        if (onVolumeChanged) onVolumeChanged(v);
    };
    playbackCoordinator_.onQueueChanged = [this]() {
        if (onQueueChanged) onQueueChanged();
    };
}

// ── playback ──────────────────────────────────────────────────────────────────

void Player::play(const Track& t) { playbackCoordinator_.play(t); }

void Player::playPlaylist(const Playlist& p) {
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;
    playbackCoordinator_.loadQueue(tracks);
    if (!tracks.empty() && tracks[0]) {
        playbackCoordinator_.loadTrack(tracks[0]->getMusicPath());
    }
}

void Player::pause() { playbackCoordinator_.pause(); }
void Player::resume() { playbackCoordinator_.resume(); }
void Player::next() { playbackCoordinator_.next(); }
void Player::previous() { playbackCoordinator_.previous(); }
void Player::seek(float seconds) { playbackCoordinator_.seek(seconds); }
void Player::setVolume(float v) { playbackCoordinator_.setVolume(v); }

// ── queue management ──────────────────────────────────────────────────────────

void Player::queueNext(const Track& t) { playbackCoordinator_.queueNext(t); }
void Player::queueLast(const Track& t) { playbackCoordinator_.queueLast(t); }
void Player::setShuffle(bool e) { playbackCoordinator_.setShuffle(e); }
void Player::setRepeat(bool e)  { playbackCoordinator_.setRepeat(e); }

std::vector<fs::path> Player::queueSnapshot() const { return playbackCoordinator_.queueSnapshot(); }
std::vector<const Track*> Player::queueTracks() const { return playbackCoordinator_.queueTracks(); }

// ── playlist management ───────────────────────────────────────────────────────

void Player::addPlaylist(Playlist p) { playlistStore_.addPlaylist(std::move(p)); }

void Player::removePlaylist(const std::string& name) {
    auto& playlists = playlistStore_.playlists();
    auto it = std::find_if(playlists.begin(), playlists.end(),
        [&](const Playlist& p) { return p.getName() == name; });
    if (it == playlists.end()) return;

    const Track* nowPlaying = playbackCoordinator_.currentTrack();
    if (nowPlaying) {
        const auto& paths = it->getPlaylistTracks();
        bool inDying = std::find(paths.begin(), paths.end(), nowPlaying->getMusicPath()) != paths.end();
        if (inDying) {
            playbackCoordinator_.stop();
            playbackCoordinator_.clearQueue();
        }
    }
    playlistStore_.removePlaylist(name);
}

void Player::addTrackToPlaylist(const std::string& name, const Track& track) { playlistStore_.addTrackToPlaylist(name, track); }
void Player::removeTrackFromPlaylist(const std::string& name, const fs::path& path) { playlistStore_.removeTrackFromPlaylist(name, path); }
void Player::moveTrackInPlaylist(const std::string& name, int from, int to) { playlistStore_.moveTrackInPlaylist(name, from, to); }

const std::vector<Playlist>& Player::playlists() const { return playlistStore_.playlists(); }

// ── persistence ───────────────────────────────────────────────────────────────

void Player::loadState() {
    SessionPersistence::loadSession(playbackCoordinator_, playlistStore_, recommendationCoordinator_, lib_);
    if (onVolumeChanged) onVolumeChanged(playbackCoordinator_.volume());
    if (onQueueChanged) onQueueChanged();
    if (onPlaylistsChanged) onPlaylistsChanged();
}

void Player::saveState() {
    SessionPersistence::saveSession(playbackCoordinator_, playlistStore_, recommendationCoordinator_);
}

// ── event loop ────────────────────────────────────────────────────────────────

void Player::update() {
    playbackCoordinator_.update();
}

// ── read state ────────────────────────────────────────────────────────────────

const Track*  Player::currentTrack()  const { return playbackCoordinator_.currentTrack(); }
PlaybackState Player::playbackState() const { return playbackCoordinator_.playbackState(); }
float         Player::volume()        const { return playbackCoordinator_.volume(); }
bool          Player::isShuffle()     const { return playbackCoordinator_.isShuffle(); }
bool          Player::isRepeat()      const { return playbackCoordinator_.isRepeat(); }
float         Player::position()      const { return playbackCoordinator_.position(); }
int           Player::currentDuration() const { return playbackCoordinator_.currentDuration(); }

std::vector<RecommendResult> Player::recommend(std::size_t limit) const {
    const Track* current = playbackCoordinator_.currentTrack();
    if (!current) return {};
    return recommendationCoordinator_.recommend(lib_, *current, limit);
}

void Player::playPlaylistStartingAt(const Playlist& p, int startIndex) {
    auto tracks = p.resolve(lib_);
    if (tracks.empty()) return;

    if (startIndex < 0 || startIndex >= (int)tracks.size()) {
        startIndex = 0;
    }

    std::vector<const Track*> shiftedTracks;
    shiftedTracks.reserve(tracks.size());

    for (int i = startIndex; i < (int)tracks.size(); ++i) {
        shiftedTracks.push_back(tracks[i]);
    }
    for (int i = 0; i < startIndex; ++i) {
        shiftedTracks.push_back(tracks[i]);
    }

    playbackCoordinator_.loadQueue(shiftedTracks);

    if (!shiftedTracks.empty() && shiftedTracks[0]) {
        playbackCoordinator_.loadTrack(shiftedTracks[0]->getMusicPath());
    }
}

void Player::playQueueIndex(std::size_t index) {
    playbackCoordinator_.playQueueIndex(index);
}

void Player::moveQueueTrackUp(std::size_t index) {
    playbackCoordinator_.moveQueueTrackUp(index);
}

void Player::moveQueueTrackDown(std::size_t index) {
    playbackCoordinator_.moveQueueTrackDown(index);
}

void Player::removeQueueTrack(std::size_t index) {
    playbackCoordinator_.removeQueueTrack(index);
}
