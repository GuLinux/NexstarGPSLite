#include "nexstar.h"
#include "logging.h"
#include <TimeLib.h>
#include "nexstar_data.h"


#define PING_DELAY 5000
#define COMMAND_IDLE 5000
#define RESPONSE_TIMEOUT 3500
#define NOT_CONNECTED_DELAY 4000

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
  DEBUG_F
  check_status();
  comms();
}

void Nexstar::comms() {
  DEBUG_F
  if(! _comm_port || _status == NotConnected || _waiting_reply) {
    return;
  }
  if(_comm_port->available()) {
    _last_command_sent = millis();
    _port.write(_comm_port->read());
  }
  if(_port.available()) {
    _comm_port->write(_port.read());
  }
}

void Nexstar::ping(Nexstar::Status next_status) {
  DEBUG_F
  _last_ping = millis();
  TRACE_F("[Nexstar] PING [status=%d]", _status);
  _port.print("Kx");
  _waiting_reply = CheckReply{
    millis(),
    "x#",
    2,
    next_status,
    NotConnected,
    true,
#ifndef DISABLE_LOGGING
    "PONG",
    "Disconnected",
#endif
  };
}


void Nexstar::check_reply() {
  DEBUG_F
//  TRACE_F("[Nexstar] Port available: %d", _port.available());
  if(!_port.available()) {
    if(millis() - _waiting_reply.time > RESPONSE_TIMEOUT) {
      TRACE("[Nexstar] Response timeout");
      _status = _waiting_reply.on_failed;
      if(_waiting_reply.close_on_failed) {
        TRACE("[Nexstar] closing port");
        _port.end();
      }
#ifndef DISABLE_LOGGING
      TRACE("[Nexstar] Communication timeout");
#endif
      _waiting_reply.reset();
    }
    return;
  }
//  TRACE("[Nexstar] Fetching reply");
  NexstarReply reply(_port);
  bool is_success = reply.equals(_waiting_reply.message, _waiting_reply.size);
//  TRACE_F("[Nexstar] Is success: %T", is_success);
  _status = is_success ? _waiting_reply.on_success : _waiting_reply.on_failed;
#ifndef DISABLE_LOGGING
  TRACE_F(
    "[Nexstar] %s [status=%d]: %s [%s]",
    is_success ? _waiting_reply.on_success_trace : _waiting_reply.on_failed_trace,
    _status,
    reply.to_hex().c_str(),
    reply.to_string().c_str()
  );
#endif
  if(!is_success && _waiting_reply.close_on_failed) {
    _port.end();
  }
  _waiting_reply.reset();
}

void Nexstar::CheckReply::reset() {
  DEBUG_F
  size = 0;
}

void Nexstar::reconnect() {
  DEBUG_F
  if(millis() - _last_ping > NOT_CONNECTED_DELAY) {
    TRACE("[Nexstar] Opening port");
    _port.begin(9600);
    ping(Connected);
  }
}

void Nexstar::check_connection() {
  DEBUG_F
  if(millis() - _last_ping > PING_DELAY && is_idle()) {
    ping(_status);
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
    _waiting_reply = CheckReply{
      millis(),
      "#",
      1,
      TimeSync,
      _status,
      false,
#ifndef DISABLE_LOGGING
      "Time successfully synced",
      "Error synchronising time",
#endif
    };
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
    _waiting_reply = CheckReply{
      millis(),
      "#",
      1,
      LocationSync,
      _status,
      false,
#ifndef DISABLE_LOGGING
      "Location successfully synced",
      "Error synchronising location",
#endif
    };
  }
}

void Nexstar::check_status() {
  DEBUG_F
  if(_waiting_reply) {
    this->check_reply();
    return;
  }
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
  return ! _waiting_reply && millis() - _last_command_sent > COMMAND_IDLE;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
