#pragma once

//#define DISABLE_LOGGING
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#include "defines.h"
#include <ArduinoLog.h>
#undef CR

#define LoggingPort Serial
#define TO_LF(s) F(s "\n")

#define LOG(method, s) Log.method(TO_LF(s))
#define LOG_F(method, s, ...) Log.method(TO_LF(s), __VA_ARGS__)

#define TRACE(s) LOG(trace, s)
#define TRACE_F(s, ...) LOG_F(trace, s, __VA_ARGS__)

#define VERBOSE(s) LOG(verbose, s)
#define VERBOSE_F(s, ...) LOG_F(verbose, s, __VA_ARGS__)

class DebugLog {
public:
    DebugLog(const char *file, int line, const char *func) : file(file), line(line), func(func) {
        VERBOSE_F("[DEBUG] ENTER %s (%s:%d)", func, file, line);
    }
    ~DebugLog() {
        VERBOSE_F("[DEBUG] EXIT %s (%s:%d)", func, file, line);
    }
private:
    const char *file;
    int line;
    const char *func;
};

#ifdef TRACE_FUNCTIONS
#define DEBUG_F DebugLog __dbg_log(__FILE__, __LINE__, __FUNCTION__);
#else
#define DEBUG_F
#endif


// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
