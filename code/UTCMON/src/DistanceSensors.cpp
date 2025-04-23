#include "DistanceSensors.h"

DistanceSensors::DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig) {
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
DevicePairInitSuccess DistanceSensors::init() {
  DevicePairInitSuccess initSuccess;
  initSuccess.left = this->left.begin(this->leftConfig.I2CAddress, this->leftConfig.DebugEnabled, &Wire);
  initSuccess.right = this->right.begin(this->rightConfig.I2CAddress, this->rightConfig.DebugEnabled, &Wire1);

  if (initSuccess.left){
    this->left.configSensor(this->leftConfig.Mode);
  }
  if (initSuccess.right){
    this->right.configSensor(this->rightConfig.Mode);
  }
  return initSuccess;
}
int DistanceSensors::getDistance(Adafruit_VL53L0X &sensor, bool debugEnabled) {
  
  sensor.rangingTest(&(this->measure), debugEnabled);
  if (measure.RangeStatus != 4) { // phase failures have incorrect data
    return measure.RangeMilliMeter;
  }
  else {
    return DISTANCE_ERROR;
  }
}
int DistanceSensors::getLeft() {
  return getDistance(this->left, this->leftConfig.DebugEnabled);
}
int DistanceSensors::getRight() {
  return getDistance(this->right, this->rightConfig.DebugEnabled);
}