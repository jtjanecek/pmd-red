#include "global.h"
#include "mgba_log.h"

#define MGBA_PRINTADDR_NEWLINE   ((volatile u32 *)0x4FFFA10)
#define MGBA_PRINTADDR_NONEWLINE ((volatile u32 *)0x4FFFA14)

// mGBA native logging interface
#define REG_MGBA_LOG_ENABLE   (*(volatile u16 *)0x04FFF780)
#define REG_MGBA_LOG_FLAGS    (*(volatile u16 *)0x04FFF700)
#define REG_MGBA_LOG_STRING   ((volatile char *)0x04FFF600)
#define MGBA_FLAG_COMMIT      0x0100
#define MGBA_ENABLE_MAGIC     0xC0DE

static void mgba_copy_to_logbuf(const char *s)
{
    // Copy up to 256 bytes including terminator; mGBA's buffer is 256.
    s32 i;
    for (i = 0; i < 255 && s[i] != '\0'; i++)
        REG_MGBA_LOG_STRING[i] = s[i];
    REG_MGBA_LOG_STRING[i] = '\0';
}

void MGBA_Init(void)
{
    REG_MGBA_LOG_ENABLE = MGBA_ENABLE_MAGIC;
}

void MGBA_Log(const char *msg)
{
    // Write pointer to the emulator's debug print address. mGBA will read the
    // string and append a newline automatically.
    *MGBA_PRINTADDR_NEWLINE = (u32)msg;
}

void MGBA_Printf(const char *fmt, ...)
{
    char sBuf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(sBuf, fmt, args);
    va_end(args);
    MGBA_Log(sBuf);
}

void MGBA_LogLevel(int level, const char *fmt, ...)
{
    char sBuf[256];
    va_list args;
    // Ensure native logging is enabled
    MGBA_Init();

    va_start(args, fmt);
    vsprintf(sBuf, fmt, args);
    va_end(args);

    mgba_copy_to_logbuf(sBuf);
    REG_MGBA_LOG_FLAGS = (u16)((level & 0xFF) | MGBA_FLAG_COMMIT);
}

void MGBA_Warnf(const char *fmt, ...)
{
    char sBuf[256];
    va_list args;
    MGBA_Init();
    va_start(args, fmt);
    vsprintf(sBuf, fmt, args);
    va_end(args);
    mgba_copy_to_logbuf(sBuf);
    REG_MGBA_LOG_FLAGS = (u16)(MGBA_LOG_WARN | MGBA_FLAG_COMMIT);
}

void MGBA_Errorf(const char *fmt, ...)
{
    char sBuf[256];
    va_list args;
    MGBA_Init();
    va_start(args, fmt);
    vsprintf(sBuf, fmt, args);
    va_end(args);
    mgba_copy_to_logbuf(sBuf);
    REG_MGBA_LOG_FLAGS = (u16)(MGBA_LOG_ERROR | MGBA_FLAG_COMMIT);
}

void MGBA_Infof(const char *fmt, ...)
{
    char sBuf[256];
    va_list args;
    MGBA_Init();
    va_start(args, fmt);
    vsprintf(sBuf, fmt, args);
    va_end(args);
    mgba_copy_to_logbuf(sBuf);
    REG_MGBA_LOG_FLAGS = (u16)(MGBA_LOG_INFO | MGBA_FLAG_COMMIT);
}
