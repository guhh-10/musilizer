#include "controller/player.hpp"
#include "model/music_directory.hpp"
#include "repository/session_persistence.hpp"
#include <algorithm>
#include <iostream>

Player::Player(Library &lib)
    : lib_(lib), playlistStore_(), recommendationCoordinator_(),
      playbackCoordinator_(lib, recommendationCoordinator_) {
  // Wire up callbacks
  playlistStore_.onPlaylistsChanged = [this]() {
    if (onPlaylistsChanged)
      onPlaylistsChanged();
  };
  recommendationCoordinator_.onRecommendationsReady = [this]() {
    if (onRecommendationsReady)
      onRecommendationsReady();
  };
  playbackCoordinator_.onTrackChanged = [this](const Track *t) {
    if (onTrackChanged)
      onTrackChanged(t);
  };
  playbackCoordinator_.onPlaybackStateChanged = [this](PlaybackState s) {
    if (onPlaybackStateChanged)
      onPlaybackStateChanged(s);
  };
  playbackCoordinator_.onVolumeChanged = [this](float v) {
    if (onVolumeChanged)
      onVolumeChanged(v);
  };
  playbackCoordinator_.onQueueChanged = [this]() {
    if (onQueueChanged)
      onQueueChanged();
  };
}

// ── file watching ─────────────────────────────────────────────────────────────

void Player::startWatching() {
  fileWatcher_.startWatching();
}

// ── playback ──────────────────────────────────────────────────────────────────

void Player::play(const Track &t) { playbackCoordinator_.play(t); }

void Player::playPlaylist(const Playlist &p) {
  auto tracks = p.resolve(lib_);
  if (tracks.empty())
    return;
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

void Player::queueNext(const Track &t) { playbackCoordinator_.queueNext(t); }
void Player::queueLast(const Track &t) { playbackCoordinator_.queueLast(t); }
void Player::setShuffle(bool e) { playbackCoordinator_.setShuffle(e); }
void Player::setRepeat(bool e) { playbackCoordinator_.setRepeat(e); }

std::vector<fs::path> Player::queueSnapshot() const {
  return playbackCoordinator_.queueSnapshot();
}
std::vector<const Track *> Player::queueTracks() const {
  return playbackCoordinator_.queueTracks();
}

// ── playlist management ───────────────────────────────────────────────────────

void Player::addPlaylist(Playlist p) {
  playlistStore_.addPlaylist(std::move(p));
}

void Player::removePlaylist(const std::string &name) {
  auto &playlists = playlistStore_.playlists();
  auto it =
      std::find_if(playlists.begin(), playlists.end(),
                   [&](const Playlist &p) { return p.getName() == name; });
  if (it == playlists.end())
    return;

  const Track *nowPlaying = playbackCoordinator_.currentTrack();
  if (nowPlaying) {
    const auto &paths = it->getPlaylistTracks();
    bool inDying = std::find(paths.begin(), paths.end(),
                             nowPlaying->getMusicPath()) != paths.end();
    if (inDying) {
      playbackCoordinator_.stop();
      playbackCoordinator_.clearQueue();
    }
  }
  playlistStore_.removePlaylist(name);
}

void Player::addTrackToPlaylist(const std::string &name, const Track &track) {
  playlistStore_.addTrackToPlaylist(name, track);
}
void Player::removeTrackFromPlaylist(const std::string &name,
                                     const fs::path &path) {
  playlistStore_.removeTrackFromPlaylist(name, path);
}
void Player::moveTrackInPlaylist(const std::string &name, int from, int to) {
  playlistStore_.moveTrackInPlaylist(name, from, to);
}

const std::vector<Playlist> &Player::playlists() const {
  return playlistStore_.playlists();
}

// ── persistence ───────────────────────────────────────────────────────────────

void Player::loadState() {
  SessionPersistence::loadSession(playbackCoordinator_, playlistStore_,
                                  recommendationCoordinator_, lib_);
  if (onVolumeChanged)
    onVolumeChanged(playbackCoordinator_.volume());
  if (onQueueChanged)
    onQueueChanged();
  if (onPlaylistsChanged)
    onPlaylistsChanged();
}

void Player::saveState() {
  SessionPersistence::saveSession(playbackCoordinator_, playlistStore_,
                                  recommendationCoordinator_);
}

// ── event loop ────────────────────────────────────────────────────────────────

void Player::update() {
  using clock = std::chrono::steady_clock;
  constexpr int  kMaxRetries = 10;
  constexpr auto kRetryDelay = std::chrono::milliseconds(500);

  playbackCoordinator_.update();

  // ── Drain new file-watcher events ────────────────────────────────────────

  fileWatcher_.poll(
      [this, &kRetryDelay](const fs::path& path) {
        // Try immediately; if the file is still being written TagLib fails —
        // park it for a retry rather than silently dropping it.
        MusicDirectory dir;
        dir.loadMetadata(path, lib_);
        if (lib_.findByPath(path)) {
          std::cout << "[FileWatcher] added: " << path << "\n";
          if (onLibraryChanged) onLibraryChanged();
        } else {
          std::cout << "[FileWatcher] add deferred (file not ready): " << path << "\n";
          pendingAdds_.push_back({ path,
                                   std::chrono::steady_clock::now() + kRetryDelay,
                                   0 });
        }
      },

      [this](const fs::path& path) {
        const Track* nowPlaying = playbackCoordinator_.currentTrack();
        if (nowPlaying &&
            nowPlaying->getMusicPath().lexically_normal() ==
                path.lexically_normal()) {
          std::cout << "[FileWatcher] currently-playing track deleted, stopping: "
                    << path << "\n";
          playbackCoordinator_.stop();
          playbackCoordinator_.clearQueue();
        }
        if (lib_.removeTrack(path)) {
          std::cout << "[FileWatcher] removed: " << path << "\n";
          if (onLibraryChanged) onLibraryChanged();
        }
      }
  );

  // ── Retry deferred additions ──────────────────────────────────────────────

  auto now = clock::now();
  int  n   = static_cast<int>(pendingAdds_.size());
  for (int i = 0; i < n; ++i) {
    auto& front = pendingAdds_.front();

    if (now < front.retryAt) {
      // Not due yet — rotate to the back.
      pendingAdds_.push_back(std::move(front));
      pendingAdds_.pop_front();
      continue;
    }

    fs::path path    = front.path;
    int      attempt = front.attempt + 1;
    pendingAdds_.pop_front();

    MusicDirectory dir;
    dir.loadMetadata(path, lib_);
    if (lib_.findByPath(path)) {
      std::cout << "[FileWatcher] added (retry " << attempt << "): " << path << "\n";
      if (onLibraryChanged) onLibraryChanged();
    } else if (attempt < kMaxRetries) {
      std::cout << "[FileWatcher] still not ready, retry "
                << attempt << "/" << kMaxRetries << ": " << path << "\n";
      pendingAdds_.push_back({ path, clock::now() + kRetryDelay, attempt });
    } else {
      std::cerr << "[FileWatcher] gave up after " << kMaxRetries
                << " attempts: " << path << "\n";
    }
  }
}


// ── read state ────────────────────────────────────────────────────────────────

const Track *Player::currentTrack() const {
  return playbackCoordinator_.currentTrack();
}
PlaybackState Player::playbackState() const {
  return playbackCoordinator_.playbackState();
}
float Player::volume() const { return playbackCoordinator_.volume(); }
bool Player::isShuffle() const { return playbackCoordinator_.isShuffle(); }
bool Player::isRepeat() const { return playbackCoordinator_.isRepeat(); }
float Player::position() const { return playbackCoordinator_.position(); }
int Player::currentDuration() const {
  return playbackCoordinator_.currentDuration();
}

std::vector<RecommendResult> Player::recommend(std::size_t limit) const {
  const Track *current = playbackCoordinator_.currentTrack();
  if (!current)
    return {};
  return recommendationCoordinator_.recommend(lib_, *current, limit);
}

void Player::playPlaylistStartingAt(const Playlist &p, int startIndex) {
  auto tracks = p.resolve(lib_);
  if (tracks.empty())
    return;

  if (startIndex < 0 || startIndex >= (int)tracks.size()) {
    startIndex = 0;
  }

  std::vector<const Track *> shiftedTracks;
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

int Player::getSamples(float *out, int count) const {
  return playbackCoordinator_.audio().getSamples(out, count);
}