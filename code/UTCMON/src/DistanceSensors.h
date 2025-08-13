#pragma once

#include <Arduino.h>
#include "hw_config.h"
#include "DistanceSensor.h"

struct DistanceStatusPair
{
  DistanceStatus left;
  DistanceStatus right;
};

class DistanceSensors
{
private:
  DistanceSensor left;
  DistanceSensor right;

public:
  DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig, bool mockSensor = false);
  DevicePairInitSuccess init();
  int getLeft();
  int getRight();
  DistanceStatusPair getSensorsStatus();
};