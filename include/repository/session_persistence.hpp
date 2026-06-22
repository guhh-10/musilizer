#pragma once
#include "controller/playback_coordinator.hpp"
#include "model/playlist_store.hpp"
#include "service/recommendation_coordinator.hpp"
#include "model/library.hpp"

namespace SessionPersistence {
    void loadSession(PlaybackCoordinator& playback, PlaylistStore& playlists, RecommendationCoordinator& recCoord, Library& lib);
    void saveSession(PlaybackCoordinator& playback, const PlaylistStore& playlists, const RecommendationCoordinator& recCoord);
}
