/**
 * @file Log.h
 * @ingroup coa_types
 * @brief Console logging macros for all engine and game code.
 *
 * Four severity levels are available: INFO, WARN, ERROR, and FATAL.
 * Every macro prefix-stamps the message with the module-relative source file
 * path and line number so that log lines are easy to trace back to their origin.
 *
 * Usage example:
 * @code
 *   LOG_INFO("Loaded mesh with %zu vertices", vertexCount);
 *   LOG_FATAL("Out of VRAM — cannot continue");  // calls abort()
 * @endcode
 *
 * @note LOG_FATAL calls std::abort() — it is reserved for unrecoverable errors
 *       where continuing execution would corrupt engine state.
 */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Types.h"

/// @cond INTERNAL — ANSI colour escape codes used only by log macros.
#define COLOR_RESET "\033[0m"
#define COLOR_INFO  "\033[32m"    // green
#define COLOR_WARN  "\033[33m"    // yellow
#define COLOR_ERROR "\033[31m"    // red
#define COLOR_FATAL "\033[1;31m"  // bold red
/// @endcond

namespace COA
{

/// Internal buffer size used by shortFile() to build the shortened path string.
constexpr i32 BUFFER_SIZE = 256;

inline const char *shortFile(const char *file)
{
    static char buffer[BUFFER_SIZE];

    // Pula até depois de "quakepg/"
    const char *ptr = std::strstr(file, "coagula-engine/");
    if (ptr != nullptr) {
        ptr += 15;
    } else {
        ptr = file;
    }

    // Detecta se é engine ou game
    const char *module = nullptr;
    if (std::strncmp(ptr, "engine/", 7) == 0) {
        module = "engine/";
    } else if (std::strncmp(ptr, "game/", 5) == 0) {
        module = "game/";
    } else {
        return ptr;  // fallback (caso esteja fora do projeto)
    }

    // Pega só o nome do arquivo (tudo depois do último '/')
    const char *last_slash = std::strrchr(ptr, '/');
    const char *base_name  = (last_slash != nullptr) ? last_slash + 1 : ptr;

    // Monta "engine/nome.cpp" ou "game/nome.cpp"
    std::snprintf(buffer, sizeof(buffer), "%s%s", module, base_name);
    return buffer;
}
/**
 * @brief Severity levels for the engine logging system.
 *
 * The enum is used internally; game code should use the macros
 * LOG_INFO / LOG_WARN / LOG_ERROR / LOG_FATAL directly.
 */
enum class LogLevel : u8
{
    Info,   ///< Informational — normal operating events.
    Warn,   ///< Warning — non-fatal, but something may be wrong.
    Error,  ///< Error — operation failed, engine may continue.
    Fatal   ///< Fatal — unrecoverable; calls abort() immediately.
};

}  // namespace COA

/**
 * @brief Log an informational message to stdout.
 * @param fmt  printf-style format string.
 * @param ...  Optional format arguments.
 */
#define LOG_INFO(fmt, ...) \
    do { \
        fprintf(stdout, \
                COLOR_INFO "[INFO] %s:%d: " fmt COLOR_RESET "\n", \
                ::COA::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

/**
 * @brief Log a warning message to stderr.
 * @param fmt  printf-style format string.
 * @param ...  Optional format arguments.
 */
#define LOG_WARN(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_WARN "[WARN] %s:%d: " fmt COLOR_RESET "\n", \
                ::COA::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

/**
 * @brief Log a non-fatal error to stderr.
 * @param fmt  printf-style format string.
 * @param ...  Optional format arguments.
 */
#define LOG_ERROR(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_ERROR "[ERROR] %s:%d: " fmt COLOR_RESET "\n", \
                ::COA::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

/**
 * @brief Log a fatal error to stderr and terminate the process immediately.
 *
 * Use this only when the engine has reached a state from which recovery is
 * impossible (e.g. GPU context lost, required asset missing). Calls std::abort().
 *
 * @param fmt  printf-style format string.
 * @param ...  Optional format arguments.
 */
#define LOG_FATAL(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_FATAL "[FATAL] %s:%d: " fmt COLOR_RESET "\n", \
                ::COA::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
        abort(); \
    } while (0)
