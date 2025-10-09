#ifndef GUARD_MGBA_LOG_H
#define GUARD_MGBA_LOG_H

#include "global.h"

// Minimal mGBA logger.
// Supports both classic no$gba-style prints (always Info) and mGBA's native
// log levels. All functions are safe with NDEBUG defined.

// Call once to enable native mGBA logging (levels). Safe to call repeatedly.
void MGBA_Init(void);

// Log levels (bit flags as used by mGBA).
// mGBA native severities (0..4)
enum
{
    MGBA_LOG_FATAL = 0,
    MGBA_LOG_ERROR = 1,
    MGBA_LOG_WARN  = 2,
    MGBA_LOG_INFO  = 3,
    MGBA_LOG_DEBUG = 4,
};

// Prints a line to the emulator log (adds newline automatically).
void MGBA_Log(const char *msg);

// printf-style convenience wrapper around MGBA_Log.
void MGBA_Printf(const char *fmt, ...);

// Native-level logging (preferred): prints with the given mGBA level.
void MGBA_LogLevel(int level, const char *fmt, ...);

// Convenience wrappers
void MGBA_Warnf(const char *fmt, ...);
void MGBA_Errorf(const char *fmt, ...);
void MGBA_Infof(const char *fmt, ...);

#endif // GUARD_MGBA_LOG_H
