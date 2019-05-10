#define GPSSerial Serial2
#define BluetoothSerial Serial3
#define NexstarSerial Serial1
#define USBSerial Serial

// Logging must be imported first
#include "logging.h"

#include "rtc.h"
#include "gps.h"
#include "nexstar.h"
#include "bluetooth.h"
#include <TimeLib.h>

#define BT_POWER_PIN PB1
#define BT_AT_MODE_PIN PB0
#define NEXSTAR_LED_PIN PA7
#define GPS_LED_PIN PA6

#define LEDS_INTERVAL 200
#define LEDS_PATTERN_SIZE 12
#define LEDS_PWM 5


RTCProvider rtcProvider;
GPS gps(GPSSerial);
Nexstar nexstar{NexstarSerial, gps, rtcProvider};
Bluetooth bluetooth(BluetoothSerial, BT_POWER_PIN, BT_AT_MODE_PIN);

Stream *commPort;

int GPSStatusLeds[][LEDS_PATTERN_SIZE] = {
    {1,0,0,0,0,0,0,0,0,0,0,0}, // NoFix
    {1,0,1,0,0,0,0,0,0,0,0,0}, // Time Fix
    {1,0,1,0,1,0,0,0,0,0,0,0}, // Fix
};

int NexstarStatusLeds[][LEDS_PATTERN_SIZE] = {
    {1,0,0,0,0,0,0,0,0,0,0,0}, // NotConnected
    {1,0,1,0,0,0,0,0,0,0,0,0}, // Connected
    {1,0,1,0,1,0,0,0,0,0,0,0}, // Time sync
    {1,0,1,0,1,0,1,0,0,0,0,0}, // Location sync
};


void setupLeds() {
    pinMode(GPS_LED_PIN, OUTPUT);
    pinMode(NEXSTAR_LED_PIN, OUTPUT);
    digitalWrite(NEXSTAR_LED_PIN, 1);
    digitalWrite(GPS_LED_PIN, 1);
    delay(500);
    digitalWrite(NEXSTAR_LED_PIN, 0);
    digitalWrite(GPS_LED_PIN, 0);
}

void setup() {
  setupLeds();
  USBSerial.begin(9600);
  Log.begin(LOG_LEVEL_VERBOSE, &LoggingPort);
  bluetooth.setup();

  TRACE("Initialising...");
  gps.begin();
  rtcProvider.setup();
}

int last_debug_print = 0;
bool rtcUpdatedFromGPS = false;

void processRTC() {
  if(gps.hasDateTime() && ! rtcUpdatedFromGPS) {
    auto date = gps.date();
    auto time = gps.time();
    rtcProvider.set_time(date.year(), date.month(), date.day(), time.hour(), time.minute(), time.second());
    rtcUpdatedFromGPS = true;
  }
  if(millis() - last_debug_print > 1000) {
    last_debug_print = millis();
    VERBOSE_F("[RTC] Time from RTC: valid=%T, %d", rtcProvider.is_valid(), rtcProvider.utc());
    VERBOSE_F("[RTC] Time from TimeLib: %d", now());
  }
}

int lastLedsWrite = 0;
int lastLedsStatus = 0;



void writeLeds() {
  if(millis() - lastLedsWrite > LEDS_INTERVAL) {
    lastLedsWrite = millis();
    int currentIndex = lastLedsStatus++ % LEDS_PATTERN_SIZE;
    analogWrite(GPS_LED_PIN, LEDS_PWM* GPSStatusLeds[gps.status()][currentIndex]); 
    analogWrite(NEXSTAR_LED_PIN, LEDS_PWM* NexstarStatusLeds[nexstar.status()][currentIndex]); 
  }
}

void check_commport() {
  if(USBSerial) {
    bluetooth.power_off();
    nexstar.set_comm_port(&USBSerial);
  } else {
    bluetooth.power_on();
    nexstar.set_comm_port(&BluetoothSerial);
  }
}

void loop() {
  check_commport();
  processRTC();
  gps.process();
  nexstar.process();
  writeLeds();
}

// vim: set shiftwidth=2 tabstop=2 expandtab:indentSize=2:tabSize=2:noTabs=true:
