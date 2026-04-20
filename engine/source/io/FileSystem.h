#pragma once

#include <filesystem>

namespace ENG
{
class FileSystem
{
public:
    std::filesystem::path GetExecutableFolder() const;
    std::filesystem::path GetAssetsFolder() const;
};
}  // namespace ENG
