#define VERSION "v0.2.2"
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
#include <U8g2lib.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>


Adafruit_VL53L0X lox_l = Adafruit_VL53L0X();
Adafruit_VL53L0X lox_r = Adafruit_VL53L0X();


DateTime dt;
UI ui(LeftBus::Display::config, RightBus::Display::config);
LightSensors light(LeftBus::LightSensor::config, RightBus::LightSensor::config);
void setup() {
  Serial.begin(CommonBus::Serial::Baudrate);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  SPI.setFrequency(CommonBus::SPI::Frequency);
  // TODO: Wire+Wire1 vs custom TwoWire instances; per-class vs global
  Wire.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL);
  Wire1.begin(RightBus::I2C::SDA, RightBus::I2C::SCL);

  ui.init(); // TODO: check init success
  ui.drawSplashScreen(VER_INFO);

  DevicePairInitSuccess lightInitSuccess = light.init();


  delay(250);
  bool lox_ok_l = lox_l.begin(LeftBus::DistanceSensor::I2CAddress, LeftBus::DistanceSensor::DebugEnabled, &Wire);
  Serial.printf("lox_l.begin() %1d\n", lox_ok_l);
  if (lox_ok_l) lox_l.configSensor(LeftBus::DistanceSensor::Mode);
  delay(250);
  bool lox_ok_r = lox_r.begin(RightBus::DistanceSensor::I2CAddress, RightBus::DistanceSensor::DebugEnabled, &Wire1);
  Serial.printf("lox_r.begin() %1d\n", lox_ok_r);
  if (lox_ok_r) lox_r.configSensor(RightBus::DistanceSensor::Mode);
  delay(250);



  ui.drawInitScreenSensor(VER_INFO, lox_ok_l, lightInitSuccess.left, lox_ok_r, lightInitSuccess.right);

  ui.drawInitScreenNetPhase1(WIFI_SSID);


  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting ...");
    delay(500);
    }
  
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

    delay(2000);
  
  dt=DateTime("Europe/Warsaw");
}



VL53L0X_RangingMeasurementData_t measure_l;
VL53L0X_RangingMeasurementData_t measure_r;
sensors_event_t event_l;
sensors_event_t event_r;
int avg_lux;
int contrast=255;
int lux_l; // only for debug phase
int lux_r; // only for debug phase
DateTimeStruct dts;
void loop() {
  dts=dt.getDateTimeStruct();


  lox_l.rangingTest(&measure_l, false);
  lox_r.rangingTest(&measure_r, false);

  lux_l = light.getLeft();
  lux_r = light.getRight();
  avg_lux = light.getAvg();
  if (avg_lux>512) contrast=255; else contrast=int(avg_lux/2); // TODO: move contrast calc to some better place + parametrize values
  Serial.printf("lux: L=%05d R=%05d AVG=%05d => contrast=%03d\n", int(event_l.light), int(event_r.light), avg_lux, contrast);
  ui.setContrast(contrast);


  ui.drawClock(dts, int(measure_l.RangeMilliMeter), int(measure_r.RangeMilliMeter), lux_l, lux_r);

  delay(250); 
}
