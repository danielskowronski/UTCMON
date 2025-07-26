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
};

struct DistanceStatus {
  uint16_t mm;
  bool triggering;
};
struct DistanceStatusPair {
  DistanceStatus left;
  DistanceStatus right;
};


class DistanceSensors 
{

private:
  Adafruit_VL53L0X left;
  Adafruit_VL53L0X right;
  DistanceSensorConfig leftConfig;
  DistanceSensorConfig rightConfig;
  int getDistance(Adafruit_VL53L0X &sensor, bool debugEnabled);
  VL53L0X_RangingMeasurementData_t measure;
  bool mockSensor;
public:
  DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig, bool mockSensor=false);
  DevicePairInitSuccess init();
  int getLeft();
  int getRight();
  DistanceStatusPair getSensorsStatus();
  static String fmtDist(DistanceStatus ds, bool preferMM = false, bool showUnit = false, bool showTriggering = false);
};