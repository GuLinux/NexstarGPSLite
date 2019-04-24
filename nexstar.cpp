#include "nexstar.h"
#include "logging.h"
#include <TimeLib.h>

#define NOT_CONNECTED_DELAY 2500
#define PING_DELAY 30000
#define COMMAND_IDLE 30000

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
  bool is_connected = send_command("Kx") == "x#";
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
  if(millis() - _last_ping > PING_DELAY && millis() - _last_command_sent > COMMAND_IDLE) {
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

  void debug() {
    TRACE_F("h:%02x,m:%02x,s:%02x,M:%02x,D:%02x,Y:%02x,t:%02x,d:%01x", hour, minute, second, month, day, year, tz, dst);
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

  void debug() {
    TRACE_F("lat: d:%02x,m:%02x,s:%02x %02x; lng: d:%02x,m:%02x,s:%02x %02x",
      latitude.degrees,
      latitude.minutes,
      latitude.seconds,
      latitude.sign,
      longitude.degrees,
      longitude.minutes,
      longitude.seconds,
      longitude.sign
    );
  }

};

template<typename T> size_t write_struct(T &s, HardwareSerial &port) {
  VERBOSE_F("[Nexstar] Writing %d bytes to port", sizeof(s));
  return port.write(reinterpret_cast<char*>(&s), sizeof(s));
}

void Nexstar::sync_time() {
  if(_rtc.is_valid()) {
    NexstarTime time(_rtc.utc(), 0, 0);
    Log.trace("[Nexstar] Syncing time");
    time.debug();
    write_struct(time, _port);
    auto reply = get_reply();
    if(reply == "#") {
      _status = TimeSync;
      TRACE("[Nexstar] Time successfully synced");
    } else {
      TRACE_F("[Nexstar] Error synchronising time: %s", reply.c_str());
   }
  }
}

void Nexstar::sync_location() {
  if(_gps.hasFix()) {
    NexstarLocation location(_gps.location().lat(), _gps.location().lng());
    Log.trace("[Nexstar] syncing location");
    location.debug();
    write_struct(location, _port);
    auto reply = get_reply();
    if(reply == "#") {
      _status = LocationSync;
      TRACE("[Nexstar] Location successfully synced");
    } else {
      TRACE_F("[Nexstar] Error synchronising location: %s", reply.c_str());
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


String Nexstar::send_command(const String &cmd) {
  _port.print(cmd);
  _last_command_sent = millis();
  return get_reply();
}

String Nexstar::get_reply() {
  uint32_t started = millis();
  char last_char;
  String output;
  while(millis() - started < 2000) {
    if(_port.available()) {
      last_char = _port.read();
      output += last_char;
      if(last_char == '#')
        break;
    }
  }
  _last_command_sent = millis();
  return output;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
