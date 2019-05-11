cmake_minimum_required(VERSION 3.13)
project(NexstarGPSLite NONE)
set(DEFAULT_BOARD bluepill)



set("DISABLE_LOGGING" On CACHE BOOL "Disable application logging to USBSerial (Default: On)")
set("LOG_LEVEL" "verbose" CACHE STRING "Log level (default: verbose, allowed values: [verbose, trace, notice, warning, error, fatal])")
set("BLUETOOTH_DEVICE_NAME" "NexstarGPS-Lite" CACHE STRING "Name for bluetooth device discovery (default: NexstarGPS-Lite)")
set("BLUETOOTH_DEVICE_PIN" "1234" CACHE STRING "Pin for bluetooth pairing (default: 1234)")
set("DEBUG_GPS" Off CACHE BOOL "Log NMEA messages (default: Off)")
set("TRACE_FUNCTIONS" Off CACHE BOOL "Enable tracing of functions for debugging (Default: Off)")

string(TOUPPER "${LOG_LEVEL}" LOG_LEVEL_H)
set(LOG_LEVEL_H "LOG_LEVEL_${LOG_LEVEL_H}")

configure_file(defines.h.in ${CMAKE_SOURCE_DIR}/defines.h)
file(
    DOWNLOAD
    https://raw.githubusercontent.com/GuLinux/GuLinux-Commons/b928073acb9a6c1e8de4bdc5c3845240dd2fa80c/Arduino/Arduino.cmake
    ${CMAKE_SOURCE_DIR}/Arduino.cmake
    EXPECTED_MD5 f8a12e01b3b15caa836e73fbab60cc2a
)

include(${CMAKE_SOURCE_DIR}/Arduino.cmake)
