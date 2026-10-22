#include "core/Log.h"

#include <cstdarg>
#include <cstdio>

namespace {
FILE* g_file = nullptr;
}

namespace Log {

void Init(const char* path) {
    g_file = std::fopen(path, "w");
}

void Info(const char* format, ...) {
    if (!g_file)
        return;
    va_list args;
    va_start(args, format);
    std::vfprintf(g_file, format, args);
    va_end(args);
    std::fputc('\n', g_file);
    std::fflush(g_file);
}

} // namespace Log
