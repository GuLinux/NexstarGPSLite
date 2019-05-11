cmake_minimum_required(VERSION 3.13)
project(NexstarGPSLite NONE)
set(DEFAULT_BOARD bluepill)



set("DISABLE_LOGGING" On CACHE BOOL "Disable application logging to USBSerial (Default: On)")
set("LOG_LEVEL" "verbose" CACHE STRING "Log level (default: verbose, allowed values: [verbose, trace, notice, warning, error, fatal])")
set("DEBUG_GPS" Off CACHE BOOL "Log NMEA messages (default: Off)")
set("TRACE_FUNCTIONS" Off CACHE BOOL "Enable tracing of functions for debugging (Default: Off)")

if(DISABLE_LOGGING)
  set(DISABLE_LOGGING_H 1)
else()
  set(DISABLE_LOGGING_H 0)
endif()

if(TRACE_FUNCTIONS)
  set(TRACE_FUNCTIONS_H 1)
else()
  set(TRACE_FUNCTIONS_H 0)
endif()


if(DEBUG_GPS)
  set(DEBUG_GPS_H 1)
else()
  set(DEBUG_GPS_H 0)
endif()

string(TOUPPER "${LOG_LEVEL}" LOG_LEVEL_H)
set(LOG_LEVEL_H "LOG_LEVEL_${LOG_LEVEL_H}")

configure_file(defines.h.in ${CMAKE_SOURCE_DIR}/defines.h)
include(../Arduino.cmake)
