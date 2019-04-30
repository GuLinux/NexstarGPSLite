#pragma once
#include <TimeLib.h>
#include "Arduino.h"
#include "logging.h"

class NexstarReply {
public:
  NexstarReply(HardwareSerial &port) {
    uint32_t started = millis();
    while(millis() - started < 2000) {
      if(port.available()) {
        _buffer[len] = port.read();
        if(_buffer[len++] == '#')
          break;
      }
    }
    _buffer[len] = 0;
  }

  void debug(bool endline=true) {
#ifndef DISABLE_LOGGING
    char log_buffer[10] = {0};
    for(size_t i=0; i<len; i++) {
      if(_buffer[i] < 31) {
        sprintf(log_buffer, "%02x []", _buffer[i]);
      } else {
        sprintf(log_buffer, "%02x [%c]", _buffer[i], _buffer[i]);
      }
      LoggingPort.print(log_buffer);
    }
    if(endline) {
      LoggingPort.write('\n');
    }
#endif 
  }

  String to_string() {
    return String(_buffer);
  }

  bool operator==(const String &s) {
    return to_string() == s;
  }

  bool equals(const char *s, size_t len) {
    if(len != this->len) {
      return false;
    }

    for(size_t i=0; i<len; i++) {
      if(s[i] != _buffer[i]) {
        return false;
      }
    }
    return true;
  }

private:
  char _buffer[256] = {0};
  size_t len = 0;
};

struct __attribute__ ((packed)) NexstarTime {
  NexstarTime(time_t timestamp, uint8_t tz, uint8_t dst) {
    tmElements_t time;
    breakTime(timestamp, time);
    hour = time.Hour;
    minute = time.Minute;
    second = time.Second;
    month = time.Month;
    day = time.Day;
    year = time.Year - 30;
    this->tz = (tz >= 0 ? tz : (256 + tz));
    this->dst = dst;
  }

  void debug(bool endline=true) {
#ifndef DISABLE_LOGGING
    char buffer[100];
    sprintf(buffer, "h:%02d,m:%02d,s:%02d,M:%02d,D:%02d,Y:%02d,tz:%02d,dst:%d", hour, minute, second, month, day, year, tz, dst);
    LoggingPort.print(buffer);
    if(endline) {
      LoggingPort.write('\n');
    }
#endif
  }

  const uint8_t ctrl = 'H';
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t month;
  uint8_t day;
  uint8_t year;
  uint8_t tz;
  uint8_t dst;
};

struct __attribute__ ((packed)) LatLng {
  uint8_t degrees;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t sign;
  LatLng(double number, uint8_t positive_value=0, uint8_t negative_value=1) {
    sign = positive_value;
    if(number < 0) {
      sign = negative_value;
      number = -number;
    }
    degrees = static_cast<uint8_t>(number);
    number = (number - static_cast<double>(degrees)) * 60;
    minutes = static_cast<uint8_t>(number);
    number = (number - static_cast<double>(minutes)) * 60;
    seconds = static_cast<uint8_t>(number);
  }
};

struct __attribute__ ((packed)) NexstarLocation {
  const uint8_t ctrl = 'W';
  const LatLng latitude;
  const LatLng longitude;

  NexstarLocation(double latitude, double longitude) : latitude(latitude), longitude(longitude) {
  }

  void debug(bool endline=true) {
#ifndef DISABLE_LOGGING
    char buffer[100];
    sprintf(
      buffer,
      "lat: d:%02d,m:%02d,s:%02d sign=%d; lng: d:%02d,m:%02d,s:%02d %d",
      latitude.degrees,
      latitude.minutes,
      latitude.seconds,
      latitude.sign,
      longitude.degrees,
      longitude.minutes,
      longitude.seconds,
      longitude.sign
    );
    LoggingPort.print(buffer);
    if(endline) {
      LoggingPort.write('\n');
    }
#endif
  }
};
