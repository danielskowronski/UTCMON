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
int LightSensors::getLeft() {
  sensors_event_t event;
  this->left.getEvent(&event);
  return event.light;
}
int LightSensors::getRight() {
  sensors_event_t event;
  this->right.getEvent(&event);
  return event.light;
}
int LightSensors::getAvg() {
  return (this->getLeft() + this->getRight()) / 2;
}