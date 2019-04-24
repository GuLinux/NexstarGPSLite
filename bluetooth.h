#pragma once

#include "Arduino.h"

class Bluetooth {
public:
  Bluetooth(HardwareSerial &port, int power_pin, int at_mode_pin);
  void setup();
  void power_on(bool at_mode=false);
  void power_off();
private:
  HardwareSerial &port;
  int power_pin;
  int at_mode_pin;
  String send_command(const String &command, int timeout=500);
  bool powered_on = false;
};

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
