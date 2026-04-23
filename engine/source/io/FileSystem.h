/**
 * @file FileSystem.h
 * @ingroup coa_io
 * @brief Asset-path resolution and file loading for the engine.
 *
 * FileSystem abstracts the difference between source-tree paths and the
 * runtime asset directory. The assets folder is resolved relative to the
 * executable using the ASSETS_DIR constant baked in via config.h at build
 * time. Access through Engine::GetFileSystem().
 *
 * @see Engine::GetFileSystem, FileReader
 */

#pragma once

#include <filesystem>

namespace COA
{

/**
 * @brief Resolves asset paths and loads files into memory.
 *
 * Owned by the Engine singleton. All relative paths passed to Load* methods
 * are resolved against GetAssetsFolder().
 */
class FileSystem
{
public:
    /**
     * @brief Return the directory containing the running executable.
     * @return Absolute path to the executable's parent directory.
     */
    std::filesystem::path GetExecutableFolder() const;

    /**
     * @brief Return the runtime assets directory.
     *
     * Resolved as ASSETS_DIR relative to the executable folder. CMake copies
     * the source-tree @c assets/ folder here post-build.
     *
     * @return Absolute path to the assets directory.
     */
    std::filesystem::path GetAssetsFolder() const;

    /**
     * @brief Load an arbitrary file into a byte buffer.
     * @param path Absolute or relative path to the file.
     * @return File contents as a vector of bytes.
     */
    std::vector<char> LoadFile(const std::filesystem::path &path);

    /**
     * @brief Load a file from the assets directory into a byte buffer.
     * @param relativePath Path relative to GetAssetsFolder().
     * @return File contents as a vector of bytes.
     */
    std::vector<char> LoadAssetFile(const std::string &relativePath);

    /**
     * @brief Load a text file from the assets directory into a string.
     * @param relativePath Path relative to GetAssetsFolder().
     * @return File contents as a UTF-8 string.
     */
    std::string LoadAssetFileText(const std::string &relativePath);
};

}  // namespace COA
