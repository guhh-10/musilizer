#pragma once
#include <chrono>
#include <deque>
#include <functional>
#include <vector>

#include "controller/playback_coordinator.hpp"
#include "model/library.hpp"
#include "model/playback_state.hpp"
#include "model/playlist.hpp"
#include "model/playlist_store.hpp"
#include "model/track.hpp"
#include "service/file_watcher.hpp"
#include "service/recommendation_coordinator.hpp"


class Player {
public:
  // Callback forwarders
  std::function<void(const Track *)> onTrackChanged;
  std::function<void(PlaybackState)> onPlaybackStateChanged;
  std::function<void(float)> onVolumeChanged;
  std::function<void()> onQueueChanged;
  std::function<void()> onRecommendationsReady;
  std::function<void()> onPlaylistsChanged;

  std::function<void()> onLibraryChanged;

private:
  Library &lib_;
  PlaylistStore playlistStore_;
  RecommendationCoordinator recommendationCoordinator_;
  PlaybackCoordinator playbackCoordinator_;
  FileWatcher fileWatcher_;

  struct PendingAdd {
    fs::path                              path;
    std::chrono::steady_clock::time_point retryAt;
    int                                   attempt = 0;
  };
  std::deque<PendingAdd> pendingAdds_;

public:
  explicit Player(Library &lib);

  // Playback (forward to PlaybackCoordinator)
  void play(const Track &t);
  void playPlaylist(const Playlist &p);
  void pause();
  void resume();
  void next();
  void previous();
  void seek(float seconds);
  void setVolume(float v);

  // Queue (forward to PlaybackCoordinator)
  void queueNext(const Track &t);
  void queueLast(const Track &t);
  void setShuffle(bool enabled);
  void setRepeat(bool enabled);
  void playQueueIndex(std::size_t index);
  void moveQueueTrackUp(std::size_t index);
  void moveQueueTrackDown(std::size_t index);
  void removeQueueTrack(std::size_t index);

  std::vector<fs::path> queueSnapshot() const;
  std::vector<const Track *> queueTracks() const;

  // Playlist management (forward to PlaylistStore)
  const std::vector<Playlist> &playlists() const;
  void addPlaylist(Playlist p);
  void removePlaylist(const std::string &name);
  void addTrackToPlaylist(const std::string &playlistName, const Track &track);
  void removeTrackFromPlaylist(const std::string &playlistName,
                               const fs::path &path);
  void moveTrackInPlaylist(const std::string &playlistName, int from, int to);
  void playPlaylistStartingAt(const Playlist &p, int startIndex);

  void startWatching();

  void saveState();
  void loadState();

  // Tick — call every frame; advances playback state and drains file events
  void update();

  // Read state (forward to PlaybackCoordinator)
  const Track *currentTrack() const;
  PlaybackState playbackState() const;
  float volume() const;
  bool isShuffle() const;
  bool isRepeat() const;
  float position() const;
  int currentDuration() const;

  // Recommendations (forward to RecommendationCoordinator)
  std::vector<RecommendResult> recommend(std::size_t limit = 10) const;
  const GenreGraphLearner &learner() const {
    return recommendationCoordinator_.learner();
  }

  // Audio samples for the waveform visualizer (reads from ring buffer).
  // `count` must be even (stereo). Returns the number of samples written.
  int getSamples(float *out, int count) const;
};