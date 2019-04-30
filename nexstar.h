#pragma once

#include "Arduino.h"
#include "gps.h"
#include "logging.h"
#include "rtc.h"

class Settings;
class Nexstar {
public:
  Nexstar(HardwareSerial &port, GPS &gps, RTCProvider &rtc);
  void set_comm_port(Stream *comm_port);
  void process();
  enum Status {
      NotConnected = 0,
      Connected = 1,
      TimeSync = 2,
      LocationSync = 3,
  };

  inline Status status() const { return _status; }

private:
  HardwareSerial &_port;
  Stream *_comm_port = nullptr;
  GPS &_gps;
  RTCProvider &_rtc;
  bool ping();
  int _last_ping = 0;
  int _last_command_sent = 0;
  Status _status = NotConnected;

  bool is_idle();
  void check_status();

  void reconnect();
  void check_connection();
  void comms();

  void sync_time();
  void sync_location();
};

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
