#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Types.h"

#define COLOR_RESET "\033[0m"
#define COLOR_INFO  "\033[32m"    // green
#define COLOR_WARN  "\033[33m"    // yellow
#define COLOR_ERROR "\033[31m"    // red
#define COLOR_FATAL "\033[1;31m"  // bold red

namespace ENG
{

constexpr i32 BUFFER_SIZE = 256;

inline const char *shortFile(const char *file)
{
    static char buffer[BUFFER_SIZE];

    // Pula até depois de "quakepg/"
    const char *ptr = std::strstr(file, "paladin-engine/");
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
enum class LogLevel : u8
{
    Info,
    Warn,
    Error,
    Fatal
};

}  // namespace ENG

#define LOG_INFO(fmt, ...) \
    do { \
        fprintf(stdout, \
                COLOR_INFO "[INFO] %s:%d: " fmt COLOR_RESET "\n", \
                ::ENG::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

#define LOG_WARN(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_WARN "[WARN] %s:%d: " fmt COLOR_RESET "\n", \
                ::ENG::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_ERROR "[ERROR] %s:%d: " fmt COLOR_RESET "\n", \
                ::ENG::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
    } while (0)

#define LOG_FATAL(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_FATAL "[FATAL] %s:%d: " fmt COLOR_RESET "\n", \
                ::ENG::shortFile(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
        abort(); \
    } while (0)
