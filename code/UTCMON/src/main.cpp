#include <Arduino.h>
#include <esp_wifi.h>
#include "hw_config.h"
#include "sys_config.h"
#include "DateTime.h"
#include "UI.h"
#include "LightSensors.h"
#include "common.h"
#include <SPI.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>
#include <string>
#include <cstdio>
#include <cstdint>
#include "debug.h"
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

void WiFiConnect(bool updateDisplay = false){
  while (true) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    logger.info(TAG_NET, "Starting WiFi connection attempt to %s", WIFI_SSID);
    if (updateDisplay) ui.drawInitScreenNetPhase1(WIFI_SSID);

    int count=0;
    while (WiFi.status() != WL_CONNECTED) {
      logger.verbose(TAG_NET, "Waiting for WiFi to connect - attempt %4d/%d", count, System::Network::ConnCheckCount);
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

    if (updateDisplay) ui.drawInitScreenNetPhase2(WiFi.localIP().toString());
    esp_wifi_set_ps(WIFI_PS_NONE); // TODO: parametrize this

    logger.info(TAG_NET, "Starting initial NTP sync to: %s", System::NTP::ServerHost);
    configTimeExtended(0, 0, System::NTP::ServerHost);
    struct tm timeinfo;
    if (updateDisplay) ui.drawInitScreenNetPhase3();
    while (!getLocalTime(&timeinfo)) {
      logger.debug(TAG_NET, "Waiting for initial NTP to finish");
      delay(1000);
    }
    checkNTP();
    if (updateDisplay) ui.drawInitScreenNetPhase4(ntpDiagnostics.lastDriftMs);
    break;
  } 
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  logger.warn(TAG_NET, "WiFi disconnected with event %d and reason", event, info.wifi_sta_disconnected.reason);
  WiFiConnect();
}


// TODO: introduce some struct to hold all displayTask state variables
int avg_lux;
int contrast=255;
int lux_l; // only for debug phase
int lux_r; // only for debug phase
DateTimeStruct dts;
DistanceStatusPair dsp;
size_t currentTz = 0;
const char* nextTimezone() {
  currentTz = (currentTz + 1) % System::DisplayModes::timezones_count;
  return System::DisplayModes::timezones[currentTz];
}

TaskHandle_t displayHandle = NULL;
void UTCMON_displayTask(void* pvParameters) {
  (void)pvParameters;
  for (;;) {
    DateTimeStruct dts = dt.getDateTimeStruct();

    checkNtpDriftIfNeeded();
    
    dsp = distance.getSensorsStatus();

    if (dsp.left.triggering && dsp.left.triggeringCounter == 1) {
      ui.setCommonDisplayMode(CommonDisplayMode((ui.getCommonDisplayMode()+1)%CommonDisplayMode::Count));
      logger.info(TAG_VBUTTONS, "Left vbutton triggered, mode changed to %s", ui.commonDisplayModeName(ui.getCommonDisplayMode()));
    }
    if (dsp.right.triggering && dsp.right.triggeringCounter == 1) {
      if (ui.getCommonDisplayMode() == CommonDisplayMode::Blank) {
        logger.info(TAG_VBUTTONS, "Right vbutton triggered but screen blanked so not taking action");
      }
      else {
        dt.changeTimezone(nextTimezone());
        dts = dt.getDateTimeStruct();
        logger.info(TAG_VBUTTONS, "Right vbutton triggered, timezone changed to %s", dts.timezone.c_str());
      }
    }


    lux_l = light.getLeft();
    lux_r = light.getRight();
    avg_lux = light.getAvg();

    if (avg_lux > 512) contrast = 255;
    else contrast = int(avg_lux / 2);
    //logger.verbose(TAG_SENSORS, "distL=%s distR=%s | luxL=%5d luxR=%5d cont=%3d", fmtDist(mm_l), fmtDist(mm_r), lux_l, lux_r, contrast); // FIXME: move to separare function / class

    ui.setContrast(contrast);
    ui.drawClock(dts, dsp, lux_l, lux_r);

    vTaskDelay(pdMS_TO_TICKS(System::Loops::DisplayTaskPeriodMs));
  }
}



// TODO: split to threads - one for clock, one for sensors
void loop() { 
  loopCheckPinMux();

  #ifdef STRESS_TEST_WIFI
  rotateWiFiPsMode();
  #endif

  if (WiFi.status() != WL_CONNECTED) {
    WiFiConnect();
  }

  //ui.setCommonDisplayMode(CommonDisplayMode((ui.getCommonDisplayMode()+1)%CommonDisplayMode::Count)); // BUG

  vTaskDelay(pdMS_TO_TICKS(System::Loops::LoopTaskPeriodMs));
}

void setup() {
  sys_config_overrides();

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
  // TODO: if any init fails, attempt system restart
  if (!(distanceInitSuccess.left) || !(lightInitSuccess.left) || !(distanceInitSuccess.right) || !(lightInitSuccess.right)) {
    logger.error(TAG_SYS, "Failed to initialize one or more sensors, restarting system -> dist_l: %d, dist_r: %d, light_l: %d, light_r: %d",
      distanceInitSuccess.left, distanceInitSuccess.right,
      lightInitSuccess.left, lightInitSuccess.right
    );
    delay(5000);
    ESP.restart();
  }

  WiFiConnect(true);
  //WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  #ifdef STRESS_TEST_U8G2
  ui.debugLoop();
  #endif

  #ifdef STRESS_TEST_CLOBBER
  startRmtClobber(5);
  #endif

  reconfigureSpiDrive(GPIO_DRIVE_CAP_3); // TODO: parametrize this via hw_config

  setupCheckPinMux();
  
  delay(2000);
  xTaskCreatePinnedToCore(
    UTCMON_displayTask,    // function
    "Display",             // name
    8192,                  // stack size in bytes
    NULL,                  // parameters
    configMAX_PRIORITIES-1,// priority
    &displayHandle,        // task handle
    1                      // run on core; 1=app, 0=system/wifi
  );

  dt=DateTime(System::DisplayModes::timezones[0]);
  ui.setDateDisplayMode(DateDisplayMode::FullAndNTP);
  //ui.setDateDisplayMode(DateDisplayMode::FullAndSensors);
}
