
#include "io/FileReader.h"

namespace ENG
{

bool FileReader::Exists()
{
    return std::filesystem::exists(m_filePath);
}

std::string FileReader::ReadToString()
{
    if (!Exists()) {
        LOG_ERROR("Could not open file: %s", m_filePath.c_str());
        return "";
    }

    std::ifstream     file(m_filePath);
    std::stringstream stream;

    stream << file.rdbuf();

    file.close();
    return stream.str();
}

std::vector<std::string> FileReader::ReadLines()
{
    if (!Exists()) {
        LOG_ERROR("Could not open file: %s", m_filePath.c_str());
        return {};
    }

    std::ifstream            file(m_filePath);
    std::string              line;
    std::vector<std::string> buf_lines;

    while (std::getline(file, line)) {
        buf_lines.push_back(line);
    }

    return buf_lines;
}

std::vector<u64> FileReader::ReadToBytes()
{
    if (!Exists()) {
        LOG_ERROR("Could not open file: %s", m_filePath.c_str());
        return {};
    }

    std::ifstream file(m_filePath, std::ios::binary);
    file.seekg(0, std::ios::end);
    u64 size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<u64> bytes(size);

    file.read(reinterpret_cast<char *>(bytes.data()), size);

    return bytes;
}

u64 FileReader::GetSize()
{
    if (!Exists()) {
        LOG_ERROR("Could not open file: %s", m_filePath.c_str());
        return {};
    }

    std::ifstream file(m_filePath, std::ios::binary);
    file.seekg(0, std::ios::end);
    u64 size = file.tellg();

    return size;
}

std::string FileReader::GetPath()
{
    if (!Exists()) {
        LOG_ERROR("Could not open file: %s", m_filePath.c_str());
        return {};
    }

    return m_filePath;
}

}  // namespace ENG
