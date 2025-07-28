#pragma once

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include "common.h"

#define DISTANCE_ERROR 9999
#define DISTANCE_MAX 2500 // values over 2.5m are deifnitely errors

struct DistanceSensorConfig {
  uint8_t I2CAddress;
  bool DebugEnabled;
  Adafruit_VL53L0X::VL53L0X_Sense_config_t Mode;

  uint64_t TriggeringHoldUs;
  uint16_t TriggeringThreshold;

  const char* alias;
};
struct DistanceStatus {
  uint16_t mm;
  bool triggering;
  uint16_t triggeringCounter;
};
class DistanceSensor {
private:
  TwoWire *i2c;
  Adafruit_VL53L0X sensor;
  VL53L0X_RangingMeasurementData_t measure;
  DistanceSensorConfig config;
  bool mockSensor;

  uint16_t triggerCounter = 0;
  bool lastTriggering = false;
  uint64_t lastTriggeringTime = 0;
public:
  DistanceSensor();
  DistanceSensor(DistanceSensorConfig config, TwoWire *i2c);
  bool init();
  int getDistance(bool debugEnabled=false);
  DistanceStatus getSensorStatus();
  static String fmtDist(DistanceStatus ds, bool preferMM = false, bool showUnit = false, bool showTriggering = false);

};