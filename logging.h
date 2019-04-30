#pragma once

//#define DISABLE_LOGGING
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#include "build/defines.h"
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


// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
