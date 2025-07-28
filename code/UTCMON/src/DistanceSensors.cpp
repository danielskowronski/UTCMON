#include "DistanceSensor.h"
#include "DistanceSensors.h"
#include "hw_config.h"
#include "Logging.h"

DistanceSensors::DistanceSensors(DistanceSensorConfig leftConfig, DistanceSensorConfig rightConfig, bool /*mockSensor*/)
  : left(leftConfig, &Wire),
    right(rightConfig, &Wire1)
{
  // mockSensor currently not supported; always initialize real sensors
}
DevicePairInitSuccess DistanceSensors::init() {
  DevicePairInitSuccess initSuccess;
  initSuccess.left = left.init();
  initSuccess.right = right.init();
  return initSuccess;
}
int DistanceSensors::getLeft() {
  return this->left.getDistance();
}
int DistanceSensors::getRight() {
  return this->right.getDistance();
}

DistanceStatusPair DistanceSensors::getSensorsStatus(){
  DistanceStatusPair status;
  status.left = this->left.getSensorStatus();
  status.right = this->right.getSensorStatus();

  logger.verbose(TAG_DIST, "L=%s R=%s", DistanceSensor::fmtDist(status.left, false, true, true), DistanceSensor::fmtDist(status.right, false, true, true));

  return status;
}