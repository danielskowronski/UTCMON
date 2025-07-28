#include "DistanceSensor.h"
#include "Logging.h"

DistanceSensor::DistanceSensor() {
  this->mockSensor = true;
}
DistanceSensor::DistanceSensor(DistanceSensorConfig config, TwoWire *i2c) {
  this->config = config;
  this->i2c = i2c;
  this->mockSensor = false;
}
bool DistanceSensor::init() {
  if (this->mockSensor) {
    return true;
  }
  
  if (!this->sensor.begin(this->config.I2CAddress, false, this->i2c)) {
    logger.error(TAG_DIST, "%s Failed to find VL53L0X sensor at address 0x%02x on I2C bus %d", this->config.alias, this->config.I2CAddress, this->i2c->getBusNum());
    return false;
  }
  else {
    logger.info(TAG_DIST, "%s VL53L0X sensor initialized at address 0x%02x on I2C bus %d", this->config.alias, this->config.I2CAddress, this->i2c->getBusNum());
    this->sensor.configSensor(this->config.Mode);
    return true;
  }
}
int DistanceSensor::getDistance(bool debugEnabled) {
  if (this->mockSensor) {
    return 2137; // mock value
  }
  
  this->sensor.rangingTest(&(this->measure), this->config.DebugEnabled || debugEnabled);
  if (measure.RangeStatus != 4) { // phase failures have incorrect data
    return measure.RangeMilliMeter;
  } else {
    return DISTANCE_ERROR;
  }
}
DistanceStatus DistanceSensor::getSensorStatus() {
  DistanceStatus status;
  status.mm = this->getDistance();
  status.triggering = false;
  status.triggeringCounter = 0;

  bool currentTriggering = (status.mm < this->config.TriggeringThreshold);

  // FIXME: improve how this logic is written
  uint64_t now = micros();
  if (currentTriggering) {
    if ((this->lastTriggeringTime!=0) && ((now - this->lastTriggeringTime) > this->config.TriggeringHoldUs)) {
      status.triggering = true;
      this->triggerCounter++;
      status.triggeringCounter = this->triggerCounter;
      logger.debug(TAG_DIST, "%s Sensor triggering detected, distance: %s, time since last: %llu us, counter: %d", this->config.alias, fmtDist(status, false, true, true).c_str(), now - this->lastTriggeringTime, this->triggerCounter);
    }
    if (!(this->lastTriggering)) {
      this->lastTriggeringTime = now;
    }
  } else {
    this->lastTriggeringTime = 0;
    this->triggerCounter = 0;
  }
  this->lastTriggering = currentTriggering;

  return status;
}
String DistanceSensor::fmtDist(DistanceStatus ds, bool preferMM, bool showUnit, bool showTriggering) {
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