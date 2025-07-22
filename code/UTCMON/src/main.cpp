#define VERSION "v0.3.0"
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
    
    ui.drawInitScreenNetPhase2();


    Serial.println("Connected");
    configTime(0, 0, "pool.ntp.org");
    struct tm timeinfo;
    ui.drawInitScreenNetPhase3();
    while (!getLocalTime(&timeinfo)) {

      Serial.println("Waiting for time sync...");
      delay(1000);
    }
    ui.drawInitScreenNetPhase4();
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
  //WiFi.begin(WIFI_SSID, WIFI_PASS);
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
}

// TODO: NTP time drift detection
// TODO: regular reboots

int avg_lux;
int contrast=255;
int lux_l; // only for debug phase
int lux_r; // only for debug phase
int mm_l; 
int mm_r;
DateTimeStruct dts;

void periodicDisplayReset(DateTimeStruct dts) {
  if (dts.timestamp - System::PeriodicDisplayReset::LastReset > System::PeriodicDisplayReset::Period) {
    Serial.printf("Periodic display reset at %10d (last: %10d, period: %d)\n", dts.timestamp, System::PeriodicDisplayReset::LastReset, System::PeriodicDisplayReset::Period);
    System::PeriodicDisplayReset::LastReset = dts.timestamp;
    ui.resetScreens();
    //ui.init();
  }
}


// TODO: split to threads - one for clock, one for sensors
void loop() {
  dts=dt.getDateTimeStruct();

  periodicDisplayReset(dts);
  if (WiFi.status() != WL_CONNECTED) {
    WiFiConnect();
  }

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
