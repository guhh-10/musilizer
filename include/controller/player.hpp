#pragma once
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

class Player {
    private:
        Library&    lib_;
        Audio       audio_;
        Queue       queue_;
        PlayHistory history_;
        std::vector<Playlist> playlists_;
        GenreGraphLearner learner_;
        Recommender       recommender_;

        const Track* nowPlaying_  = nullptr;

        // Low-level: load audio + update nowPlaying_. Never touches history or
        // the queue. Returns true on success.
        bool loadAudio(const fs::path& path);

        // High-level: loadAudio + push to history. Used for forward navigation.
        void loadTrack(const fs::path& path);

        // Record a genre transition and refresh the recommender graph.
        void observeTransition(const Track* prev,
                               const Track* next,
                               bool         skipped);

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

        // Persistence
        void saveState();
        void loadState();

        // Called on a tick / event loop
        void update();

        // Read state for the UI
        const Track* currentTrack() const;
        float        volume()       const;
        bool         isShuffle()    const;
        bool         isRepeat()     const;

        // Recommendation — uses nowPlaying_, not the queue cursor
        std::vector<RecommendResult> recommend(std::size_t limit = 10) const;
        const GenreGraphLearner& learner() const { return learner_; }
};