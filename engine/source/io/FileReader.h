/**
 * @file FileReader.h
 * @ingroup coa_io
 * @brief Thin wrapper around std::ifstream for reading a single file.
 *
 * Construct with an absolute or relative path, check Exists(), then call
 * whichever read method fits the caller's needs. FileSystem::LoadFile() and
 * FileSystem::LoadAssetFileText() are higher-level alternatives that handle
 * asset-path resolution automatically.
 *
 * @see FileSystem
 */

#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Log.h"
#include "Types.h"

namespace COA
{

/**
 * @brief Read helper bound to a single file path.
 *
 * All read methods open, read, and close the file on each call — there is no
 * persistent file handle. Check Exists() before reading to avoid error logs.
 */
class FileReader
{
private:
    std::string m_filePath;  ///< Absolute or relative path to the target file.

public:
    /**
     * @brief Bind the reader to the given file path.
     * @param path Absolute or relative path to the file.
     */
    explicit FileReader(const std::string &path)
        : m_filePath(path)
    {
    }

    /**
     * @brief Check whether the file exists on disk.
     * @return True if the path resolves to a regular file.
     */
    bool Exists();

    /**
     * @brief Read the file line-by-line.
     * @return Vector of lines, each without a trailing newline.
     */
    std::vector<std::string> ReadLines();

    /**
     * @brief Read the entire file into a single string.
     * @return File contents as a UTF-8 string.
     */
    std::string ReadToString();

    /**
     * @brief Read the file as raw bytes.
     * @return Each byte of the file represented as a u64 value.
     */
    std::vector<u64> ReadToBytes();

    /**
     * @brief Return the file size in bytes.
     * @return Number of bytes, or 0 if the file cannot be opened.
     */
    u64 GetSize();

    /**
     * @brief Return the path this reader was constructed with.
     * @return The path string passed to the constructor.
     */
    std::string GetPath();
};

}  // namespace COA
