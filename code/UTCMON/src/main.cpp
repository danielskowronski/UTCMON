#define VERSION "v0.2.1"
#ifndef BUILD_DATE
  #define BUILD_DATE "YYYY-MM-DD"
#endif
#define VER_INFO VERSION " " BUILD_DATE

#include <Arduino.h>

#include "hw_config.h"
#include "DateTime.h"

#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

TwoWire I2C_l = TwoWire(LeftBus::I2C::Bus);
TwoWire I2C_r = TwoWire(RightBus::I2C::Bus);

Adafruit_VL53L0X lox_l = Adafruit_VL53L0X();
Adafruit_VL53L0X lox_r = Adafruit_VL53L0X();

Adafruit_TSL2561_Unified tsl_l = Adafruit_TSL2561_Unified(LeftBus::LightSensor::I2CAddress);
Adafruit_TSL2561_Unified tsl_r = Adafruit_TSL2561_Unified(RightBus::LightSensor::I2CAddress);


DateTime dt;
UI ui(LeftBus::Display::config, RightBus::Display::config);
void setup() {
  Serial.begin(CommonBus::Serial::Baudrate);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  SPI.setFrequency(CommonBus::SPI::Frequency);

  ui.drawSplashScreen(VER_INFO);


  Wire.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL);
  Wire1.begin(RightBus::I2C::SDA, RightBus::I2C::SCL);
  I2C_l.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL, LeftBus::I2C::Frequency);
  I2C_r.begin(RightBus::I2C::SDA, RightBus::I2C::SCL, RightBus::I2C::Frequency); 

  delay(250);
  bool lox_ok_l = lox_l.begin(LeftBus::DistanceSensor::I2CAddress, LeftBus::DistanceSensor::DebugEnabled, &Wire);
  Serial.printf("lox_l.begin() %1d\n", lox_ok_l);
  if (lox_ok_l) lox_l.configSensor(LeftBus::DistanceSensor::Mode);
  delay(250);
  bool lox_ok_r = lox_r.begin(RightBus::DistanceSensor::I2CAddress, RightBus::DistanceSensor::DebugEnabled, &Wire1);
  Serial.printf("lox_r.begin() %1d\n", lox_ok_r);
  if (lox_ok_r) lox_r.configSensor(RightBus::DistanceSensor::Mode);
  delay(250);

  bool tsl_ok_l = tsl_l.begin(&Wire);
  Serial.printf("tsl_l.begin() %1d\n", tsl_ok_l);
  delay(250);
  bool tsl_ok_r = tsl_r.begin(&Wire1);
  Serial.printf("tsl_r.begin() %1d\n", tsl_ok_r);
  delay(250);


  ui.drawInitScreenSensor(VER_INFO, lox_ok_l, tsl_ok_l, lox_ok_r, tsl_ok_r);

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

DateTimeStruct dts;
void loop() {
  dts=dt.getDateTimeStruct();


  lox_l.rangingTest(&measure_l, false);
  lox_r.rangingTest(&measure_r, false);

  tsl_l.getEvent(&event_l);
  tsl_r.getEvent(&event_r);
  avg_lux = int((int(event_l.light)+int(event_r.light))/2);
  if (avg_lux>512) contrast=255; else contrast=int(avg_lux/2);
  Serial.printf("lux: L=%05d R=%05d AVG=%05d => contrast=%03d\n", int(event_l.light), int(event_r.light), avg_lux, contrast);
  ui.setContrast(contrast);


  ui.drawClock(dts, int(measure_l.RangeMilliMeter), int(measure_r.RangeMilliMeter), int(event_l.light), int(event_r.light));

  delay(250); 
}
