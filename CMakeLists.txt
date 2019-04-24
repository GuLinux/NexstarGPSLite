cmake_minimum_required(VERSION 3.13)
project(NexstarGPSLite NONE)
set(DEFAULT_BOARD mapleMini)

configure_file(GuLinux-Commons/Arduino/GuLinux-Arduino-Utils/logger.h ${CMAKE_CURRENT_SOURCE_DIR}/ COPYONLY)
configure_file(GuLinux-Commons/Arduino/GuLinux-Arduino-Utils/logger.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ COPYONLY)
configure_file(GuLinux-Commons/Arduino/GuLinux-Arduino-Utils/singleton.h ${CMAKE_CURRENT_SOURCE_DIR}/ COPYONLY)
include(../Arduino.cmake)

