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
    const std::filesystem::path& directory,
    std::function<void(const std::filesystem::path&, fileStatus)> onChange)
{
    if (!std::filesystem::is_directory(directory))
    {
        LogWarning("Specified path is not a directory!");
        return;
    }

    std::thread dispatch(
        &DirectoryWatcher::AddDirectory, this, directory, onChange);
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
        for (auto& [path, files, onChange] : m_Directories)
        {
            for (const auto& [filepath, last_modified] : files)
            {
                if (!std::filesystem::exists(filepath))
                {
                    std::cout << "file erased: " << filepath << '\n';
                    onChange(path, fileStatus::eDeleted);
                    files.erase(filepath);
                }
            }
            for (const auto& file :
                 std::filesystem::recursive_directory_iterator(path))
            {
                const std::filesystem::path path = file.path();
                const std::filesystem::file_time_type last_write_time =
                    file.last_write_time();
                const auto it = files.find(path.string());

                if (it == files.end())
                {
                    std::cout << "file created: " << path.string() << '\n';
                    onChange(path, fileStatus::eCreated);
                    files.insert({path.string(), last_write_time});
                }
                else if (it->second != last_write_time)
                {
                    std::cout << "File modified: " << path.string() << '\n';
                    onChange(path, fileStatus::eModified);
                    it->second = last_write_time;
                }
            }
        }
        m_DirectoryMutex.unlock();
    }
}

void DirectoryWatcher::AddDirectory(
    const std::filesystem::path& directory,
    std::function<void(const std::filesystem::path&, fileStatus)> onChange)
{
    m_DirectoryMutex.lock();
    std::unordered_map<std::string, std::filesystem::file_time_type> files;
    for (const auto& file :
         std::filesystem::recursive_directory_iterator(directory))
    {
        files.insert({file.path().string(), file.last_write_time()});
    }
    m_Directories.push_back({directory, files, onChange});
    m_DirectoryMutex.unlock();
}