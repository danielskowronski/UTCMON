#define VERSION "v0.3.2"
#ifndef BUILD_DATE
  #define BUILD_DATE "YYYY-MM-DD"
#endif
#define VER_INFO VERSION " " BUILD_DATE

#include <Arduino.h>

#include "hw_config.h"
#include "DateTime.h"
#include "UI.h"
#include "LightSensors.h"
#include "common.h"

#include <SPI.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>

#include <Wire.h>
#include "TimeSync.h"


void checkNtpDriftIfNeeded() {
  time_t now = time(nullptr);
  if (now - ntpDiagnostics.lastDriftCheck >= System::NTP::CheckPeriodS) {
    Serial.printf("Checking NTP drift at %10d (last: %10d, period: %d)\n", now, ntpDiagnostics.lastDriftCheck, System::NTP::CheckPeriodS);
    checkNTP();
    // ntpDiagnostics.lastDriftCheck is set in checkNTP()
  }
}

DateTime dt;
UI ui(LeftBus::Display::config, RightBus::Display::config);
LightSensors light(LeftBus::LightSensor::config, RightBus::LightSensor::config);
DistanceSensors distance(LeftBus::DistanceSensor::config, RightBus::DistanceSensor::config, false);


void WiFiConnect(){
  while (true) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    ui.drawInitScreenNetPhase1(WIFI_SSID);

    int count=0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connecting ...");
      delay(500);
      count++;
      if (count > 120) {
        Serial.println("Failed to connect to WiFi, breaking...");
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect to WiFi, retrying...");
      continue;
    }
    Serial.printf(WiFi.localIP().toString().c_str());
    Serial.printf("\n");
    
    ui.drawInitScreenNetPhase2(WiFi.localIP().toString());


    Serial.println("Connected");
    //configTime(0, 0, "pool.ntp.org");
    configTimeExtended(0, 0, System::NTP::ServerHost);
    struct tm timeinfo;
    ui.drawInitScreenNetPhase3();
    while (!getLocalTime(&timeinfo)) {

      Serial.println("Waiting for time sync...");
      delay(1000);
    }
    checkNTP();
    ui.drawInitScreenNetPhase4(ntpDiagnostics.lastDriftMs);
    break;
  } 
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFiConnect();
}

void setup() {
  Serial.begin(CommonBus::Serial::Baudrate);
  SPI.begin(CommonBus::SPI::SCK, CommonBus::SPI::MISO, CommonBus::SPI::MOSI);
  SPI.setFrequency(CommonBus::SPI::Frequency);
  // TODO: Wire+Wire1 vs custom TwoWire instances; per-class vs global
  Wire.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL);
  Wire1.begin(RightBus::I2C::SDA, RightBus::I2C::SCL);

  ui.init(); // TODO: check init success
  ui.drawSplashScreen(VER_INFO);

  DevicePairInitSuccess lightInitSuccess = light.init();
  DevicePairInitSuccess distanceInitSuccess = distance.init();

  ui.drawInitScreenSensor(VER_INFO, distanceInitSuccess.left, lightInitSuccess.left, distanceInitSuccess.right, lightInitSuccess.right);

  WiFiConnect();
  //WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);


  delay(2000);
  
  dt=DateTime("Europe/Warsaw");
  ui.setDateDisplayMode(DateDisplayMode::FullAndNTP);
}


int avg_lux;
int contrast=255;
int lux_l; // only for debug phase
int lux_r; // only for debug phase
int mm_l; 
int mm_r;
DateTimeStruct dts;
uint64_t lastDisplayReset = 0;

void periodicDisplayReset() {
  time_t now = time(nullptr);
  if (now - lastDisplayReset > System::PeriodicDisplayReset::Period) {
    Serial.printf("Periodic display reset at %10d (last: %10d, period: %d)\n", now, lastDisplayReset, System::PeriodicDisplayReset::Period);
    lastDisplayReset = now;
    ui.resetScreens();
    //ui.init();
  }
}


// TODO: split to threads - one for clock, one for sensors
void loop() {
  if (WiFi.status() != WL_CONNECTED)  WiFiConnect();

  dts=dt.getDateTimeStruct();

  periodicDisplayReset();
  checkNtpDriftIfNeeded();


  mm_l = distance.getLeft();
  mm_r = distance.getRight();

  lux_l = light.getLeft();
  lux_r = light.getRight();
  avg_lux = light.getAvg();

  if (avg_lux>512) contrast=255; else contrast=int(avg_lux/2); // TODO: move contrast calc to some better place + parametrize values
  //Serial.printf("lux: L=%05d R=%05d AVG=%05d => contrast=%03d\n", lux_l, lux_r, avg_lux, contrast);
  ui.setContrast(contrast);

  ui.drawClock(dts, mm_l, mm_r, lux_l, lux_r);

  delay(250); 
}
