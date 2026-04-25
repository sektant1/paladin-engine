#include "Log.h"

#include <cstdio>

namespace COA
{

namespace
{
std::deque<LogEntry>   g_entries;

const char *LevelTag(LogLevel lv)
{
    switch (lv)
    {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "?";
}

const char *LevelColor(LogLevel lv)
{
    switch (lv)
    {
        case LogLevel::Info:  return kAnsiInfo;
        case LogLevel::Warn:  return kAnsiWarn;
        case LogLevel::Error: return kAnsiError;
        case LogLevel::Fatal: return kAnsiFatal;
    }
    return "";
}
}  // namespace

void LogEmit(LogLevel level, const char *file, int line, const char *fmt, ...)
{
    char    msg[kLogMessageBufSize];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    const char *tag      = LevelTag(level);
    const char *color    = LevelColor(level);
    const char *fileTrim = shortFile(file);

    FILE *stream = (level == LogLevel::Info) ? stdout : stderr;
    std::fprintf(stream, "%s[%s] %s:%d: %s%s\n", color, tag, fileTrim, line, msg, kAnsiReset);

    char full[kLogFullBufSize];
    std::snprintf(full, sizeof(full), "[%s] %s:%d: %s", tag, fileTrim, line, msg);

    g_entries.push_back({level, std::string(full)});
    while (g_entries.size() > kMaxLogEntries)
    {
        g_entries.pop_front();
    }
}

const std::deque<LogEntry> &LogGetEntries()
{
    return g_entries;
}

void LogClear()
{
    g_entries.clear();
}

}  // namespace COA
