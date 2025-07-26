#include "DistanceSensors.h"
#include "hw_config.h"
#include "Logging.h"

DistanceSensors::DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig, bool mockSensor) {
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
  this->mockSensor = mockSensor;
}
DevicePairInitSuccess DistanceSensors::init() {
  DevicePairInitSuccess initSuccess;
  if (this->mockSensor) {
    initSuccess.left = true;
    initSuccess.right = true;
  }
  else {
    initSuccess.left = this->left.begin(this->leftConfig.I2CAddress, this->leftConfig.DebugEnabled, &Wire);
    initSuccess.right = this->right.begin(this->rightConfig.I2CAddress, this->rightConfig.DebugEnabled, &Wire1);

    if (initSuccess.left){
      this->left.configSensor(this->leftConfig.Mode);
    }
    if (initSuccess.right){
      this->right.configSensor(this->rightConfig.Mode);
    }
  }
  return initSuccess;
}
int DistanceSensors::getDistance(Adafruit_VL53L0X &sensor, bool debugEnabled) {
  if (this->mockSensor) {
    return 2137;
  }
  else {
    sensor.rangingTest(&(this->measure), debugEnabled);
    if (measure.RangeStatus != 4) { // phase failures have incorrect data
      return measure.RangeMilliMeter;
    }
    else {
      return DISTANCE_ERROR;
    }
  }
}
int DistanceSensors::getLeft() {
  return getDistance(this->left, this->leftConfig.DebugEnabled);
}
int DistanceSensors::getRight() {
  return getDistance(this->right, this->rightConfig.DebugEnabled);
}
String DistanceSensors::fmtDist(DistanceStatus ds, bool preferMM, bool showUnit, bool showTriggering) {
  String unit = showUnit ? (preferMM ? "mm" : "cm") : "";
  String triggerMark = showTriggering ? (ds.triggering ? "!" : ".") : "";

  if (ds.mm > DISTANCE_MAX || ds.mm == DISTANCE_ERROR) {
    String ret = preferMM ? "____" : "_____";
    if (ds.mm == DISTANCE_ERROR) ret = preferMM ? "----" : "-----";
    
    return ret+unit+triggerMark;
  }

  char buffer[11];
  if (preferMM) {
    sprintf(buffer, "%4d%s%s", ds.mm, unit.c_str(), triggerMark.c_str());
  }
  else {
    sprintf(buffer, "%3d.%d%s%s", ds.mm / 10, ds.mm % 10, unit.c_str(), triggerMark.c_str());
  }
  return String(buffer);
}
DistanceStatusPair DistanceSensors::getSensorsStatus(){
  DistanceStatusPair status;
  status.left.mm = this->getLeft();
  status.right.mm = this->getRight();
  status.left.triggering = (status.left.mm < LeftBus::DistanceSensor::TriggeringThreshold);
  status.right.triggering = (status.right.mm < RightBus::DistanceSensor::TriggeringThreshold);

  logger.verbose(TAG_DIST, "L=%s R=%s", this->fmtDist(status.left, false, true, true), this->fmtDist(status.right, false, true, true));

  return status;
}