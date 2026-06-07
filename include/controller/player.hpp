#pragma once
#include <vector>

#include "model/library.hpp"
#include "model/audio.hpp"
#include "model/queue.hpp"
#include "model/play_history.hpp"
#include "model/playlist.hpp"
#include "model/track.hpp"
#include "repository/persistence.hpp"
#include "config.hpp"

class Player {
    private:
        Library&    lib_;
        Audio       audio_;
        Queue       queue_;
        PlayHistory history_;
        std::vector<Playlist> playlists_;

        void loadTrack(const fs::path& path);

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
};