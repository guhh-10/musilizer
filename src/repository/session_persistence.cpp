#include "repository/session_persistence.hpp"
#include "repository/persistence.hpp"

namespace SessionPersistence {

void loadSession(PlaybackCoordinator& playback, PlaylistStore& playlists, RecommendationCoordinator& recCoord, Library& lib) {
    float volume; 
    bool shuffle, repeat;
    Persistence::loadSettings(volume, shuffle, repeat);
    
    playback.setVolumeInternal(volume);
    playback.setShuffleInternal(shuffle);
    playback.setRepeatInternal(repeat);
    
    playlists.setPlaylists(Persistence::loadPlaylists(lib));
    Persistence::loadHistory(playback.history(), lib);
    
    GenreGraphLearner learner = recCoord.learner();
    Persistence::loadLearner(learner);
    recCoord.setLearner(learner);
}

void saveSession(PlaybackCoordinator& playback, const PlaylistStore& playlists, const RecommendationCoordinator& recCoord) {
    Persistence::saveSettings(playback.volume(), playback.isShuffle(), playback.isRepeat());
    Persistence::savePlaylists(playlists.playlists());
    Persistence::saveHistory(playback.history());
    Persistence::saveLearner(recCoord.learner());
}

}
