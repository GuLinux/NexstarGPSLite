#include "bluetooth.h"
#include "logging.h"


Bluetooth::Bluetooth(HardwareSerial &port, int power_pin, int at_mode_pin) : port(port), power_pin(power_pin), at_mode_pin(at_mode_pin) {
}

void Bluetooth::setup() {
  pinMode(power_pin, OUTPUT);
  pinMode(at_mode_pin, OUTPUT);

  digitalWrite(at_mode_pin, 1); // Set AT mode
  power_on(true);

  TRACE("Initialising BT");
  send_command("AT");
  send_command("AT+NAME=\"NexstarGPS-Lite\"");
  send_command("AT+PSWD=\"1234\"");
  digitalWrite(at_mode_pin, 0);
  send_command("AT+RESET");
  power_off();
}

void Bluetooth::power_on(bool at_mode) {
  if(powered_on) {
    return;
  }
  digitalWrite(power_pin, 1);
  delay(200); 
  port.begin(at_mode ? 38400 : 9600);
  powered_on = true;
}

void Bluetooth::power_off() {
  if(!powered_on) {
    return;
  }
  digitalWrite(power_pin, 0);
  port.end();
  powered_on = false;
}

String Bluetooth::send_command(const String &command, int timeout) {
  port.print(command);
  port.print(F("\r\n"));
  VERBOSE_F(">>> %s", command.c_str());
  int watch_port_started = millis();
  while(!port.available() && millis() - watch_port_started < timeout) {
    delay(10);
  }
  String reply;
  while(port.available())
    reply += static_cast<char>(port.read());
  VERBOSE_F("<<< %s", reply.c_str());
  return reply;
}


/* vim: set shiftwidth=2 tabstop=2 expandtab smarttab : */
