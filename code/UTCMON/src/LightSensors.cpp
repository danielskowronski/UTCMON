#include "LightSensors.h"

LightSensors::LightSensors(LightSensorConfig leftConfig, LightSensorConfig rightConfig) : left(leftConfig.I2CAddress), right(rightConfig.I2CAddress) {
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
DevicePairInitSuccess LightSensors::init() {
  DevicePairInitSuccess initSuccess;
  initSuccess.left = this->left.begin(&Wire);
  initSuccess.right = this->right.begin(&Wire1);

  if (initSuccess.left){
    this->left.enableAutoRange(leftConfig.AutoRangeEnabled);
    this->left.setIntegrationTime(leftConfig.IntegrationTime);
    this->left.setGain(leftConfig.Gain);
  }
  if (initSuccess.right){
    this->right.enableAutoRange(rightConfig.AutoRangeEnabled);
    this->right.setIntegrationTime(rightConfig.IntegrationTime);
    this->right.setGain(rightConfig.Gain);
  }
  return initSuccess;
}
int LightSensors::getLight(Adafruit_TSL2561_Unified &sensor) {
  sensors_event_t event;
  sensor.getEvent(&event);
  if (event.light) {
    return event.light;
  }
  else {
    return 0;
  }
}
int LightSensors::getLeft() {
  return getLight(this->left);
}
int LightSensors::getRight() {
  return getLight(this->right);
}
int LightSensors::getAvg() {
  return (this->getLeft() + this->getRight()) / 2;
}