#pragma once

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include "common.h"

#define DISTANCE_ERROR 9999

struct DistanceSensorConfig {
  uint8_t I2CAddress;
  bool DebugEnabled;
  Adafruit_VL53L0X::VL53L0X_Sense_config_t Mode;
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

public:
DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig);
  DevicePairInitSuccess init();
  int getLeft();
  int getRight();
};