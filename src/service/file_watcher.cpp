#include <algorithm>
#include <iostream>

#include "service/file_watcher.hpp"

static bool isMp3(const std::string& filename) {
    if (filename.size() < 4) return false;
    std::string ext = filename.substr(filename.size() - 4);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".mp3";
}

void FileWatcher::Listener::handleFileAction(
    efsw::WatchID /*watchid*/,
    const std::string& dir,
    const std::string& filename,
    efsw::Action action,
    const std::string& oldFilename) 
{
    if (!isMp3(filename)) {
        if (action != efsw::Actions::Moved || !isMp3(oldFilename))
            return;
    }

    fs::path dirPath(dir);

    switch (action) {
        case efsw::Actions::Add:
            owner_.enqueue({ FileEvent::Added, dirPath / filename });
            break;

        case efsw::Actions::Delete:
            owner_.enqueue({ FileEvent::Removed, dirPath / filename });
            break;

        case efsw::Actions::Modified:
            owner_.enqueue({ FileEvent::Removed, dirPath / filename });
            owner_.enqueue({ FileEvent::Added,   dirPath / filename });
            break;

        case efsw::Actions::Moved:
            if (isMp3(oldFilename))
                owner_.enqueue({ FileEvent::Removed, dirPath / oldFilename });
            if (isMp3(filename))
                owner_.enqueue({ FileEvent::Added,   dirPath / filename });
            break;

        default:
            break;
    }
}

FileWatcher::~FileWatcher() {
    if (watchId_ >= 0)
        watcher_.removeWatch(watchId_);
}

void FileWatcher::startWatching() {
    if (watchId_ >= 0) return;

    if (!fs::exists(config::MUSIC_DIR)) {
        std::cerr << "[FileWatcher] music directory does not exist yet: "
                  << config::MUSIC_DIR << " — watching skipped\n";
        return;
    }

    watchId_ = watcher_.addWatch(
        config::MUSIC_DIR.string(),
        &listener_,
        true
    );

    if (watchId_ < 0) {
        std::cerr << "[FileWatcher] failed to add watch for "
                  << config::MUSIC_DIR << "\n";
        return;
    }

    watcher_.watch(); 
    std::cout << "[FileWatcher] watching " << config::MUSIC_DIR << "\n";
}

void FileWatcher::enqueue(FileChange change) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push(std::move(change));
}

void FileWatcher::poll(const OnAdded& onAdded, const OnRemoved& onRemoved) {
    std::queue<FileChange> local;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        std::swap(local, queue_);
    }

    while (!local.empty()) {
        FileChange& change = local.front();
        switch (change.type) {
            case FileEvent::Added:
            case FileEvent::Modified:
                if (onAdded) onAdded(change.path);
                break;
            case FileEvent::Removed:
                if (onRemoved) onRemoved(change.path);
                break;
        }
        local.pop();
    }
}