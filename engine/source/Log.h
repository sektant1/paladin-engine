/**
 * @file Log.h
 * @ingroup coa_types
 * @brief Console logging macros for all engine and game code.
 *
 * Four severity levels: INFO, WARN, ERROR, FATAL. Each macro prefixes the
 * message with the module-relative source file path and line number, prints
 * to stdout/stderr with ANSI colour, and pushes the formatted line into a
 * ring buffer the in-game editor's Console panel reads.
 */

#pragma once

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>

#include "Constants.h"
#include "Types.h"

namespace COA
{

inline const char *shortFile(const char *file)
{
    static char buffer[kLogPrefixBufSize];

    const char *ptr = std::strstr(file, "coagula-engine/");
    if (ptr != nullptr) {
        ptr += 15;
    } else {
        ptr = file;
    }

    const char *module = nullptr;
    if (std::strncmp(ptr, "engine/", 7) == 0) {
        module = "engine/";
    } else if (std::strncmp(ptr, "game/", 5) == 0) {
        module = "game/";
    } else {
        return ptr;
    }

    const char *last_slash = std::strrchr(ptr, '/');
    const char *base_name  = (last_slash != nullptr) ? last_slash + 1 : ptr;

    std::snprintf(buffer, sizeof(buffer), "%s%s", module, base_name);
    return buffer;
}

enum class LogLevel : u8
{
    Info,
    Warn,
    Error,
    Fatal
};

struct LogEntry
{
    LogLevel    level;
    std::string text;
};

/// Emit a formatted log line to stdout/stderr and the ring buffer.
void LogEmit(LogLevel level, const char *file, int line, const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

/// Snapshot of the in-memory log buffer (bounded ring).
const std::deque<LogEntry> &LogGetEntries();

/// Clear the in-memory log buffer.
void LogClear();

}  // namespace COA

#define LOG_INFO(fmt, ...)  ::COA::LogEmit(::COA::LogLevel::Info,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ::COA::LogEmit(::COA::LogLevel::Warn,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::COA::LogEmit(::COA::LogLevel::Error, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) do { ::COA::LogEmit(::COA::LogLevel::Fatal, __FILE__, __LINE__, fmt, ##__VA_ARGS__); std::abort(); } while (0)
