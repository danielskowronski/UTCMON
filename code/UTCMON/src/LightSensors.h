#pragma once

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "common.h"

struct LightSensorConfig
{
  uint8_t I2CAddress;
  bool DebugEnabled;
  bool AutoRangeEnabled;
  tsl2561IntegrationTime_t IntegrationTime;
  tsl2561Gain_t Gain;
};

class LightSensors
{

private:
  Adafruit_TSL2561_Unified left;
  Adafruit_TSL2561_Unified right;
  LightSensorConfig leftConfig;
  LightSensorConfig rightConfig;
  int getLight(Adafruit_TSL2561_Unified &sensor);

public:
  LightSensors(LightSensorConfig leftConfig, LightSensorConfig rightConfig);
  DevicePairInitSuccess init();
  int getLeft();
  int getRight();
  int getAvg();
};