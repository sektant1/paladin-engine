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

    bool                     exists();
    std::vector<std::string> readLines();
    std::string              readToString();
    std::vector<u64>         readToBytes();
    u64                      getSize();
    std::string              getPath();
};

}  // namespace ENG
