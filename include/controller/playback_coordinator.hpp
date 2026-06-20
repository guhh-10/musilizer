#pragma once

#include "model/audio.hpp"
#include "model/queue.hpp"
#include "model/play_history.hpp"
#include "model/playback_state.hpp"
#include "model/library.hpp"
#include "model/track.hpp"
#include "service/recommendation_coordinator.hpp"
#include <functional>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class PlaybackCoordinator {
private:
    Library& lib_;
    Audio audio_;
    Queue queue_;
    PlayHistory history_;
    
    RecommendationCoordinator& recommendationCoordinator_;

    const Track* nowPlaying_ = nullptr;
    PlaybackState playbackState_ = PlaybackState::Stopped;

    void emitTrackChanged() const;
    void emitPlaybackStateChanged() const;
    void emitVolumeChanged(float v) const;
    void emitQueueChanged() const;

    void pickAndEnqueueRecommendation();

public:
    std::function<void(const Track*)>   onTrackChanged;
    std::function<void(PlaybackState)>  onPlaybackStateChanged;
    std::function<void(float)>          onVolumeChanged;
    std::function<void()>               onQueueChanged;

    PlaybackCoordinator(Library& lib, RecommendationCoordinator& recCoord);

    bool loadAudio(const fs::path& path);
    void loadTrack(const fs::path& path);

    void play(const Track& t);
    void pause();
    void resume();
    void next();
    void previous();
    void seek(float seconds);
    void setVolume(float v);

    void queueNext(const Track& t);
    void queueLast(const Track& t);
    void setShuffle(bool enabled);
    void setRepeat(bool enabled);
    void playQueueIndex(std::size_t index);
    void moveQueueTrackUp(std::size_t index);
    void moveQueueTrackDown(std::size_t index);
    void removeQueueTrack(std::size_t index);

    void loadQueue(const std::vector<const Track*>& tracks);
    void clearQueue();

    std::vector<fs::path> queueSnapshot() const;
    std::vector<const Track*> queueTracks() const;

    const Track*  currentTrack()   const;
    PlaybackState playbackState()  const;
    float         volume()         const;
    bool          isShuffle()      const;
    bool          isRepeat()       const;
    float         position()       const;
    int           currentDuration() const;

    void update();

    Audio& audio() { return audio_; }
    Queue& queue() { return queue_; }
    PlayHistory& history() { return history_; }
    
    void stop();

    void setVolumeInternal(float v);
    void setShuffleInternal(bool enabled);
    void setRepeatInternal(bool enabled);
};
