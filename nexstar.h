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
  void ping(Status next_status);
  void check_reply();
  int _last_ping = 0;
  int _last_command_sent = 0;

  struct CheckReply {
    uint32_t time;
    char message[50];
    size_t size;
    Status on_success;
    Status on_failed;
    bool close_on_failed;
#ifndef DISABLE_LOGGING
    char on_success_trace[256];
    char on_failed_trace[256];
#endif
    operator bool() const { return size > 0; }
    void reset();
  };

  CheckReply _waiting_reply;

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
