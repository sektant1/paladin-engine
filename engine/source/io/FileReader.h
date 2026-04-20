#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Log.h"
#include "Types.h"

namespace ENG
{

class FileReader
{
private:
    std::string m_filePath;

public:
    explicit FileReader(const std::string &path)
        : m_filePath(path)
    {
    }

    bool                     Exists();
    std::vector<std::string> ReadLines();
    std::string              ReadToString();
    std::vector<u64>         ReadToBytes();
    u64                      GetSize();
    std::string              GetPath();
};

}  // namespace ENG
