#pragma once
#include <Arduino.h>
#include <stdarg.h>

namespace Log {

// Tune these to your needs
#ifndef LOG_QUEUE_LEN
#define LOG_QUEUE_LEN   64        // max messages buffered
#endif
#ifndef LOG_MSG_MAX
#define LOG_MSG_MAX     256       // max bytes per message (incl. '\0')
#endif

enum DropPolicy : uint8_t { DROP_NEWEST = 0, DROP_OLDEST = 1 };

void init(DropPolicy policy = DROP_OLDEST,
          UBaseType_t taskPrio = 1,
          uint32_t taskStack = 2048,
          BaseType_t core = 0);

// Thread-safe, non-blocking. If full, applies the configured drop policy.
void printf(const char* fmt, ...) __attribute__((format(printf,1,2)));

// Optional: flush remaining messages (blocks until queue empty).
void flush(uint32_t timeoutMs = 1000);

// Convenience macro: keeps callsites short
#define LOGF(...) ::Log::printf(__VA_ARGS__)

} // namespace Log
