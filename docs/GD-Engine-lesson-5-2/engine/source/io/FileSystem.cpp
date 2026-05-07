#include "io/FileSystem.h"

#include "config.h"

#if defined _WIN32
#include <windows.h>
#elif defined (__APPLE__)
#include <mach-o/dyld.h>
#elif defined (__linux__)
#include <unistd.h>
#include <limits.h>
#endif

#include <fstream>

namespace eng
{
    std::filesystem::path FileSystem::GetExecutableFolder() const
    {
#if defined _WIN32
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(NULL, buf, MAX_PATH);
        return std::filesystem::path(buf).remove_filename();
#elif defined (__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string tmp(size, '\0');
        _NSGetExecutablePath(tmp.data(), &size);
        return std::filesystem::weakly_canonical(std::filesystem::path(tmp)).remove_filename();
#elif defined (__linux__)
        return std::filesystem::weakly_canonical(std::filesystem::read_symlink("/proc/self/exe")).remove_filename();
#else
        return std::filesystem::current_path();
#endif
    }

    std::filesystem::path FileSystem::GetAssetsFolder() const
    {
#if defined (ASSETS_ROOT)
        auto path = std::filesystem::path(std::string(ASSETS_ROOT));
        if (std::filesystem::exists(path))
        {
            return path;
        }
#endif
        return std::filesystem::weakly_canonical(GetExecutableFolder() / "assets");
    }

    std::vector<char> FileSystem::LoadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            return {};
        }

        auto size = file.tellg();
        file.seekg(0);

        std::vector<char> buffer(size);

        if (!file.read(buffer.data(), size))
        {
            return {};
        }

        return buffer;
    }

    std::vector<char> FileSystem::LoadAssetFile(const std::string& relativePath)
    {
        return LoadFile(GetAssetsFolder() / relativePath);
    }

    std::string FileSystem::LoadAssetFileText(const std::string& relativePath)
    {
        auto buffer = LoadAssetFile(relativePath);
        return std::string(buffer.begin(), buffer.end());
    }
}