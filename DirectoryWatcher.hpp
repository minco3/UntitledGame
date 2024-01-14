#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr const std::chrono::duration delay = std::chrono::milliseconds(500);

class DirectoryWatcher
{
public:
    DirectoryWatcher();
    void SubscribeToDirectory(const std::filesystem::path& directory);

private:
    void start();
    void AddDirectory(const std::filesystem::path& directory);
    std::vector<std::filesystem::path> m_Directories;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_Files;
    std::mutex m_DirectoryMutex;
    std::thread m_WatcherThread;
};