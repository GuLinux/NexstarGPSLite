#include "nexstar.h"
#include "logging.h"
#include <TimeLib.h>
#include "nexstar_data.h"

#define NOT_CONNECTED_DELAY 2500
#define PING_DELAY 15000
#define COMMAND_IDLE 15000

Nexstar::Nexstar(HardwareSerial &port, GPS &gps, RTCProvider &rtc) : _port(port), _gps(gps), _rtc(rtc) {
}

void Nexstar::set_comm_port(Stream *comm_port) {
  if(this->_comm_port != comm_port) {
    this->_comm_port = comm_port;
#ifndef DISABLE_LOGGING
    TRACE_F("[Nexstar] Serial port: %s", _comm_port == &Serial ? "USB Serial" : "Bluetooth");
#endif
  }
}

void Nexstar::process() {
  check_status();
  comms();
}

void Nexstar::comms() {
  if(_comm_port->available()) {
    _last_command_sent = millis();
    _port.write(_comm_port->read());
  }
  if(_port.available()) {
    _comm_port->write(_port.read());
  }
}

bool Nexstar::ping() {
  _port.print("Kx");
  NexstarReply reply(_port);
  
  bool is_connected = reply.equals("x#", 2);
  if(!is_connected) {
    _status = NotConnected;
    Log.trace("[Nexstar] Disconnected - ");
#if LOG_LEVEL >= LOG_LEVEL_TRACE
    reply.debug();
#endif
  }
  _last_ping = millis();
  return is_connected;
}

void Nexstar::reconnect() {
  _port.begin(9600);
  if(millis() - _last_ping > NOT_CONNECTED_DELAY && ping()) {
    TRACE("[Nexstar] Connection established");
    _status = Connected;
  } else {
    _port.end();
  }
}

void Nexstar::check_connection() {
  if(millis() - _last_ping > PING_DELAY && is_idle()) {
    TRACE_F("[Nexstar] Connected: %T", ping());
  }
}

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
  return millis() - _last_command_sent > COMMAND_IDLE;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
