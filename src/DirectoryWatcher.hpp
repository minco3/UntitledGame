#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr const std::chrono::duration delay = std::chrono::milliseconds(500);

enum class fileStatus
{
    eCreated,
    eModified,
    eDeleted
};

class DirectoryWatcher
{
    struct Directory
    {
        std::filesystem::path path;
        std::unordered_map<std::string, std::filesystem::file_time_type> files;
        std::function<void(const std::filesystem::path&, fileStatus)> onChange;
    };

public:
    DirectoryWatcher();
    ~DirectoryWatcher();
    void SubscribeToDirectory(
        const std::filesystem::path& directory,
        std::function<void(const std::filesystem::path&, fileStatus)> onChange);

private:
    void start();
    void AddDirectory(
        const std::filesystem::path& directory,
        std::function<void(const std::filesystem::path&, fileStatus)> onChange);
    std::vector<Directory> m_Directories;
    std::mutex m_DirectoryMutex;
    bool m_Running;
    std::thread m_WatcherThread;
};