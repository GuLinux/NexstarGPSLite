#include <Arduino.h>
#include <RTClock.h>
#pragma once

class RTCProvider {
public:
  RTCProvider();
  void setup();
  time_t utc() const;
  void set_time(time_t time);
  void set_time(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  bool is_valid() const;
private:
  mutable RTClock rtclock;
};

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
