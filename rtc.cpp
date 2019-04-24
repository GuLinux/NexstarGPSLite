#include "rtc.h"
#include <TimeLib.h>

#define REFERENCE_UNIX_TIMESTAMP 1546300800 // 01/01/2019 @ 12:00am (UTC) 

time_t syncProvider();

RTCProvider *_rtc_provider_instance = nullptr;

RTCProvider::RTCProvider() : rtclock(RTCSEL_LSE) {
    _rtc_provider_instance = this;
}

void RTCProvider::setup() {
  setSyncProvider(syncProvider);
  setSyncInterval(2);
}

time_t RTCProvider::utc() const {
  return rtclock.getTime();
}

void RTCProvider::set_time(time_t time) {
  rtclock.setTime(time);
}


void RTCProvider::set_time(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  tmElements_t _time{
    second, minute, hour,
    0,
    day, month, CalendarYrToTm(year),
  };
  this->set_time(makeTime(_time));
}

time_t syncProvider() {
  if(_rtc_provider_instance) {
    return _rtc_provider_instance->utc();
  }
  return 0;
}

bool RTCProvider::is_valid() const {
    return utc() > REFERENCE_UNIX_TIMESTAMP;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
