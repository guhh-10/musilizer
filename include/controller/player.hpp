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
        GenreGraphLearner learner_;   // owns counts + prior
        Recommender       recommender_;
 
        const Track* nowPlaying_  = nullptr;
        bool         trackSkipped_ = false;  // set by manual next()/previous()
 
        void loadTrack(const fs::path& path);
 
        // Record a transition observation and refresh the recommender graph.
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
        void update();   // checks audio.hasTrackEnded(), advances queue

        // Read state for the UI
        const Track* currentTrack() const;
        float        volume()       const;
        bool         isShuffle()    const;
        bool         isRepeat()     const;

                // Recommendation
        std::vector<RecommendResult> recommend(std::size_t limit = 10) const;
        const GenreGraphLearner& learner() const { return learner_; }
};