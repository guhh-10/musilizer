#pragma once
#include <functional>
#include <vector>

#include "model/library.hpp"
#include "model/audio.hpp"
#include "model/queue.hpp"
#include "model/play_history.hpp"
#include "model/playlist.hpp"
#include "model/track.hpp"
#include "repository/persistence.hpp"
#include "service/recommender.hpp"
#include "config.hpp"

// Playback state

enum class PlaybackState {
    Stopped,   // nothing loaded
    Playing,
    Paused,
};

// Player

class Player {
    public:
        // Event callbacks
        //
        // Register before calling loadState() or any playback method so that
        // initial state changes are observed. All callbacks fire on the same
        // thread that called the mutating method (or update()), so GUI
        // frameworks that require main-thread updates (Qt, etc.) are safe as
        // long as update() is driven from the main thread via a timer.
        //
        // Any callback left as nullptr is silently skipped.

        // Fired whenever nowPlaying_ changes (new track, stop, or null).
        // Argument is nullptr when playback stops.
        std::function<void(const Track*)> onTrackChanged;

        // Fired when playback transitions between Playing / Paused / Stopped.
        std::function<void(PlaybackState)> onPlaybackStateChanged;

        // Fired after setVolume(). Argument is the new volume in [0, 1].
        std::function<void(float)> onVolumeChanged;

        // Fired after any queue mutation: load, shuffle toggle, repeat toggle,
        // queueNext, queueLast.
        std::function<void()> onQueueChanged;

        // Fired after a genre transition has been observed and the recommender
        // graph has been refreshed. The GUI can call recommend() immediately
        // after receiving this.
        std::function<void()> onRecommendationsReady;

        // Fired after addPlaylist / removePlaylist so the playlist panel can
        // refresh without polling.
        std::function<void()> onPlaylistsChanged;

    private:
        Library&    lib_;
        Audio       audio_;
        Queue       queue_;
        PlayHistory history_;
        std::vector<Playlist> playlists_;
        GenreGraphLearner learner_;
        Recommender       recommender_;

        const Track*  nowPlaying_     = nullptr;
        PlaybackState playbackState_  = PlaybackState::Stopped;

        // Low-level: load audio + update nowPlaying_. Never touches history or
        // the queue. Fires onTrackChanged and onPlaybackStateChanged on success.
        // Returns true on success.
        bool loadAudio(const fs::path& path);

        // High-level: loadAudio + push to history. Used for forward navigation.
        void loadTrack(const fs::path& path);

        // Record a genre transition, refresh the recommender, fire
        // onRecommendationsReady.
        void observeTransition(const Track* prev,
                               const Track* next,
                               bool         skipped);

        // Helpers to fire callbacks safely (no-op if the callback is null).
        void emitTrackChanged()          const;
        void emitPlaybackStateChanged()  const;
        void emitVolumeChanged(float v)  const;
        void emitQueueChanged()          const;
        void emitRecommendationsReady()  const;
        void emitPlaylistsChanged()      const;

    public:
        explicit Player(Library& lib);

        // Playback
        void play(const Track& t);
        void playPlaylist(const Playlist& p);
        void pause();
        void resume();
        void next();
        void previous();
        void seek(float seconds);
        void setVolume(float v);

        // Queue management
        void queueNext(const Track& t);
        void queueLast(const Track& t);
        void setShuffle(bool enabled);
        void setRepeat(bool enabled);

        // Playlist management
        void addPlaylist(Playlist p);
        void removePlaylist(const std::string& name);
        // Add a track to an existing playlist by name. No-op if the playlist
        // does not exist. Fires onPlaylistsChanged on success.
        void addTrackToPlaylist(const std::string& playlistName, const Track& track);
        const std::vector<Playlist>& playlists() const;

        // Persistence
        void saveState();
        void loadState();

        // Event loop tick
        // Call periodically (e.g. every 100 ms from a GUI timer). Checks for
        // natural track-end and advances the queue.
        void update();

        // Read state
        const Track*  currentTrack()   const;
        PlaybackState playbackState()  const;
        float         volume()         const;
        bool          isShuffle()      const;
        bool          isRepeat()       const;
        float         position()       const;

        // Recommendations
        std::vector<RecommendResult> recommend(std::size_t limit = 10) const;
        const GenreGraphLearner& learner() const { return learner_; }
};