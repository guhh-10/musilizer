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

enum class PlaybackState {
    Stopped,
    Playing,
    Paused,
};

class Player {
    public:
        std::function<void(const Track*)>   onTrackChanged;
        std::function<void(PlaybackState)>  onPlaybackStateChanged;
        std::function<void(float)>          onVolumeChanged;
        std::function<void()>               onQueueChanged;
        std::function<void()>               onRecommendationsReady;
        std::function<void()>               onPlaylistsChanged;

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

        bool loadAudio(const fs::path& path);
        void loadTrack(const fs::path& path);
        void observeTransition(const Track* prev, const Track* next, bool skipped);

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

        // Queue
        void queueNext(const Track& t);
        void queueLast(const Track& t);
        void setShuffle(bool enabled);
        void setRepeat(bool enabled);
        void playQueueIndex(std::size_t index);

        // Returns ordered paths currently in the queue (current track first).
        std::vector<fs::path> queueSnapshot() const;
        // Resolves queue paths to Track pointers via the library (nullptr if not found).
        std::vector<const Track*> queueTracks() const;

        // Playlist management
        const std::vector<Playlist>& playlists() const;
        void addPlaylist(Playlist p);
        void removePlaylist(const std::string& name);
        void addTrackToPlaylist(const std::string& playlistName, const Track& track);
        void removeTrackFromPlaylist(const std::string& playlistName, const fs::path& path);
        void moveTrackInPlaylist(const std::string& playlistName, int from, int to);
        void playPlaylistStartingAt(const Playlist& p, int startIndex);

        // Persistence
        void saveState();
        void loadState();

        // Tick
        void update();

        // Read state
        const Track*  currentTrack()   const;
        PlaybackState playbackState()  const;
        float         volume()         const;
        bool          isShuffle()      const;
        bool          isRepeat()       const;
        float         position()       const;
        int           currentDuration() const;  // seconds; 0 if nothing loaded

        // Recommendations
        std::vector<RecommendResult> recommend(std::size_t limit = 10) const;
        const GenreGraphLearner& learner() const { return learner_; }
};