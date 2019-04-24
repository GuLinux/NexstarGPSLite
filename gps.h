#pragma once
#include "Arduino.h"
#include <TinyGPS++.h>

class GPS {
public:
    GPS(HardwareSerial &port);
    void begin();
    void process();
    void sleep();
    void resume();
    inline TinyGPSLocation location() const { return gps.location; }
    inline TinyGPSDate date() const { return gps.date; }
    inline TinyGPSTime time() const { return gps.time; }
    inline bool hasFix() const { return gps.location.isValid(); }
    inline bool hasDateTime() const { return date().isValid() && date().year() >= 2019; }
    
    enum Status {
        NoFix = 0,
        TimeFix = 1,
        Fix = 2,
    };
    Status status() const;

private:
    HardwareSerial &port;
    TinyGPSPlus gps;
    bool _suspended = false;
    Status _status = NoFix;
};

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
