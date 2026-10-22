#pragma once

// Tiny file logger. Writes ArmageddonPlus.log in the game folder; flushed on every line
// so the log survives a crash (the most useful moment to read it).
namespace Log {

void Init(const char* path);
void Info(const char* format, ...);

} // namespace Log
