#pragma once

#include <efsw/efsw.hpp>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

#include "config.hpp"

enum class FileEvent {
    Added,
    Removed,
    Modified,   
};

struct FileChange {
    FileEvent   type;
    fs::path    path;
};

class FileWatcher {
    private:
        class Listener : public efsw::FileWatchListener {
        public:
            explicit Listener(FileWatcher& owner) : owner_(owner) {}
    
            void handleFileAction(efsw::WatchID watchid,
                                  const std::string& dir,
                                  const std::string& filename,
                                  efsw::Action action,
                                  const std::string& oldFilename = "") override;
        private:
            FileWatcher& owner_;
        };
    
        void enqueue(FileChange change);
    
        efsw::FileWatcher watcher_;
        Listener          listener_{ *this };
        efsw::WatchID     watchId_ = -1;
    
        std::mutex             queue_mutex_;
        std::queue<FileChange> queue_;
        
public:
    using OnAdded   = std::function<void(const fs::path&)>;
    using OnRemoved = std::function<void(const fs::path&)>;

    FileWatcher() = default;
    ~FileWatcher();

    FileWatcher(const FileWatcher&)            = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    void startWatching();
    void poll(const OnAdded& onAdded, const OnRemoved& onRemoved);
};