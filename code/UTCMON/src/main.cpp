#define VERSION "v0.3.3"
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

#include "Logging.h"

void checkNtpDriftIfNeeded() {
  time_t now = time(nullptr);
  if (now - ntpDiagnostics.lastDriftCheck >= System::NTP::CheckPeriodS) {
    // no extra logging, checkNTP handles that
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
    logger.info(TAG_NET, "Starting WiFi connection attempt to %s", WIFI_SSID);
    ui.drawInitScreenNetPhase1(WIFI_SSID);

    int count=0;
    while (WiFi.status() != WL_CONNECTED) {
      logger.debug(TAG_NET, "Waiting for WiFi to connect - attempt %4d/%d", count, System::Network::ConnCheckCount);
      delay(System::Network::ConnCheckPeriodMs);
      count++;
      if (count > System::Network::ConnCheckCount) {
        //logger.warn(TAG_NET, "Failed to connect to WiFi");
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      logger.warn(TAG_NET, "Failed to connect to WiFi");
      continue;
    }
    logger.info(TAG_NET, "WiFi connected, obtained IP address: %s", WiFi.localIP().toString().c_str());

    ui.drawInitScreenNetPhase2(WiFi.localIP().toString());


    logger.info(TAG_NET, "Starting initial NTP sync to: %s", System::NTP::ServerHost);
    configTimeExtended(0, 0, System::NTP::ServerHost);
    struct tm timeinfo;
    ui.drawInitScreenNetPhase3();
    while (!getLocalTime(&timeinfo)) {
      logger.debug(TAG_NET, "Waiting for initial NTP to finish");
      delay(1000);
    }
    checkNTP();
    ui.drawInitScreenNetPhase4(ntpDiagnostics.lastDriftMs);
    break;
  } 
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  logger.warn(TAG_NET, "WiFi disconnected with event %d and reason", event, info.wifi_sta_disconnected.reason);
  WiFiConnect();
}
void setup() {
  Serial.begin(CommonBus::Serial::Baudrate);
  while(!Serial); logger.forwardTo(&Serial);
  logger.setLevel(System::Logging::Level); 
  Mycila::Logger::setTimeProvider([]() -> uint32_t {
    return time(nullptr);
  });

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
    logger.debug(TAG_DISP_RST, "Periodic display reset (last: %10d, period: %d)", lastDisplayReset, System::PeriodicDisplayReset::Period);
    lastDisplayReset = now;
    ui.resetScreens();
    //ui.init();
  }
}

String fmtDist(int mm) {
  if (mm > 8000) return "-----";
  char buffer[8];
  sprintf(buffer, "%3d.%d", mm / 10, mm % 10);
  return String(buffer);
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
  logger.verbose(TAG_SENSORS, "distL=%s distR=%s | luxL=%5d luxR=%5d cont=%3d", fmtDist(mm_l), fmtDist(mm_r), lux_l, lux_r, contrast);

  ui.setContrast(contrast);

  ui.drawClock(dts, mm_l, mm_r, lux_l, lux_r);

  delay(250); 
}
