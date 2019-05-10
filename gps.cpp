#include "gps.h"
#include "logging.h"

//#define DEBUG_GPS

namespace {
  static const char sleepMessage[] = {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};
}



GPS::GPS(HardwareSerial &port) : port(port) {
}

void GPS::begin() {
  TRACE("[GPS] Initialising GPS");
  port.begin(9600);
  TRACE_F("[GPS] Initialised TinyGPS++: %s", gps.libraryVersion());
}
int lastDebugPrinted = 0;

String last_sentence;

void GPS::process() {
  int incoming = 0;
  while (port.available() > 1) {
    incoming = port.read();
#ifndef DISABLE_LOGGING
#ifdef DEBUG_GPS
    char c = static_cast<char>(incoming);
    if(c == '\r') {
      last_sentence.trim();
      VERBOSE_F("[GPS] Last sentence: %s", last_sentence.c_str());
      last_sentence = String();
    } else {
      last_sentence += c;
    }
#endif
#endif
    gps.encode(incoming);
  }

  if(hasFix()) {
    _status = Fix;
  } else if(hasDateTime()) {
    _status = TimeFix;
  } else {
    _status = NoFix;
  }

  #ifndef DISABLE_LOGGING
  if(millis() - lastDebugPrinted > 1000) {
    lastDebugPrinted = millis();
    Log.verbose(F("[GPS] Location Fix: "));
    if(gps.location.isValid()) {
      VERBOSE_F("%F lat, %F lng", gps.location.lat(), gps.location.lng());
    } else {
      VERBOSE("no"); 
    }
    Log.verbose(F("[GPS] Date/Time: "));
    if(gps.date.isValid()) {
      VERBOSE_F("%d-%d-%dT%d:%d:%d.%d", 
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second(),
        gps.time.centisecond()
      );
    } else {
      VERBOSE("no");
    }
  }
  #endif

}

void GPS::sleep() {
  VERBOSE("Suspending GPS");
  for (uint8_t i = 0; i < sizeof(sleepMessage); i++)
    port.write(sleepMessage[i]);
  delay(1000);
  _suspended = true;

}

void GPS::resume() {
  VERBOSE("Resuming GPS");
  delay(500);
  for (int i = 0; i < 10; i++)
    port.write("\xFF");
  delay(500);
  _suspended = false;
}

GPS::Status GPS::status() const {
    return _status;
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
