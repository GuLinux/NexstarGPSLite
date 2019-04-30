#include "nexstar.h"
#include "logging.h"
#include <TimeLib.h>

#define NOT_CONNECTED_DELAY 2500
#define PING_DELAY 30000
#define COMMAND_IDLE 5000

Nexstar::Nexstar(HardwareSerial &port, GPS &gps, RTCProvider &rtc) : _port(port), _gps(gps), _rtc(rtc) {
}

void Nexstar::set_comm_port(Stream *comm_port) {
  _port.begin(9600);
  this->_comm_port = comm_port;
}

void Nexstar::process() {
  check_status();
  comms();
}

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

void Nexstar::comms() {
  while(_comm_port->available()) {
    _last_command_sent = millis();
    _port.write(_comm_port->read());
  }
  while(_port.available()) {
    _comm_port->write(_port.read());
  }
}

bool Nexstar::ping() {
  _port.print("Kx");
  NexstarReply reply(_port);
  
  bool is_connected = reply.equals("x#", 2);
  if(!is_connected && _status != NotConnected) {
    _status = NotConnected;
    TRACE("[Nexstar] Disconnected");
  }
  _last_ping = millis();
  return is_connected;
}

void Nexstar::reconnect() {
  if(millis() - _last_ping > NOT_CONNECTED_DELAY && ping()) {
    TRACE("[Nexstar] Connection established");
    _status = Connected;
  }
}

void Nexstar::check_connection() {
  if(millis() - _last_ping > PING_DELAY && is_idle()) {
    TRACE_F("[Nexstar] Connected: %T", ping());
  }
}

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
    sprintf(buffer, "h:%02x,m:%02x,s:%02x,M:%02x,D:%02x,Y:%02x,t:%02x,d:%01x", hour, minute, second, month, day, year, tz, dst);
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
  LatLng(double number) {
    sign = 1;
    if(number < 0) {
      sign = -1;
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
      "lat: d:%02x,m:%02x,s:%02x %02x; lng: d:%02x,m:%02x,s:%02x %02x",
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
  }
#endif
};

template<typename T> size_t write_struct(T &s, HardwareSerial &port) {
  VERBOSE_F("[Nexstar] Writing %d bytes to port", sizeof(s));
  return port.write(reinterpret_cast<char*>(&s), sizeof(s));
}

void Nexstar::sync_time() {
  if(_rtc.is_valid() && is_idle()) {
    NexstarTime time(_rtc.utc(), 0, 0);
    Log.trace("[Nexstar] Syncing time ");
#if LOG_LEVEL >= LOG_LEVEL_TRACE
    time.debug();
#endif
    write_struct(time, _port);
    NexstarReply reply(_port);
    if(reply.equals("#", 1)) {
      _status = TimeSync;
      TRACE("[Nexstar] Time successfully synced");
    } else {
      Log.trace("[Nexstar] Error synchronising time: ");
#if LOG_LEVEL >= LOG_LEVEL_TRACE
      reply.debug();
#endif
   }
  }
}

void Nexstar::sync_location() {
  if(_gps.hasFix() && is_idle()) {
    NexstarLocation location(_gps.location().lat(), _gps.location().lng());
    Log.trace("[Nexstar] syncing location ");
#if LOG_LEVEL >= LOG_LEVEL_TRACE
    location.debug();
#endif
    write_struct(location, _port);
    NexstarReply reply(_port);
    if(reply == "#") {
      _status = LocationSync;
      TRACE("[Nexstar] Location successfully synced");
    } else {
      Log.trace("[Nexstar] Error synchronising location: ");
#if LOG_LEVEL >= LOG_LEVEL_TRACE
      reply.debug();
#endif
    }
  }
}


void Nexstar::check_status() {
  switch(_status) {
    case NotConnected:
      reconnect();
      break;
    case Connected:
      check_connection();
      sync_time();
      break;
    case TimeSync:
      check_connection();
      sync_location();
      break;
    default:
      break;
  }
}


bool Nexstar::is_idle() {
  return millis() - _last_command_sent < COMMAND_IDLE;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
