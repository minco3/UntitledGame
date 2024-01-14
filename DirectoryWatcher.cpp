#include "DirectoryWatcher.hpp"
#include "Log.hpp"

DirectoryWatcher::DirectoryWatcher()
    : m_Running(true), m_WatcherThread(&DirectoryWatcher::start, this)
{
}

DirectoryWatcher::~DirectoryWatcher()
{
    m_Running = false;
    m_WatcherThread.join();
}

void DirectoryWatcher::SubscribeToDirectory(
    const std::filesystem::path& directory)
{
    if (!std::filesystem::is_directory(directory))
    {
        LogWarning("Specified path is not a directory!");
        return;
    }

    std::thread dispatch(&DirectoryWatcher::AddDirectory, this, directory);
    dispatch.detach();
}

void DirectoryWatcher::start()
{
    while (m_Running)
    {
        std::this_thread::sleep_for(delay);
        if (!m_DirectoryMutex.try_lock())
        {
            continue;
        }
        for (const auto& [path, last_modified] : m_Files)
        {
            if (!std::filesystem::exists(path))
            {
                std::cout << "file erased: " << path << '\n';
                m_Files.erase(path);
            }
        }
        for (const auto& directory : m_Directories)
        {
            for (const auto& file :
                 std::filesystem::recursive_directory_iterator(directory))
            {
                const std::filesystem::path path = file.path();
                const std::filesystem::file_time_type last_write_time =
                    file.last_write_time();
                const auto it = m_Files.find(path);

                if (it == m_Files.end())
                {
                    std::cout << "file created: " << path.string() << '\n';
                    m_Files.insert({path.string(), last_write_time});
                }
                else if (it->second != last_write_time)
                {
                    std::cout << "File modified: " << path.string() << '\n';
                    it->second = last_write_time;
                }
            }
        }
        m_DirectoryMutex.unlock();
    }
}

void DirectoryWatcher::AddDirectory(const std::filesystem::path& directory)
{
    m_DirectoryMutex.lock();
    m_Directories.push_back(directory);
    for (const auto& file :
         std::filesystem::recursive_directory_iterator(directory))
    {
        m_Files.insert({file.path().string(), file.last_write_time()});
    }
    m_DirectoryMutex.unlock();
}