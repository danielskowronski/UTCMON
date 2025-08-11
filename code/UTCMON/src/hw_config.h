#pragma once
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include "LightSensors.h"
#include "DistanceSensor.h"

// Display configuration for U8g2 screens
struct DisplayConfig {
  uint32_t Frequency;
  uint8_t CS;
  uint8_t DC;
  uint8_t RESET;
};
#include "LightSensors.h"
#include "DistanceSensor.h"

#define kHz 1000
#define MHz 1000000
#define SPI_FREQUENCY 20*MHz

// FIXME: LeftBust and RightBust should inherit from some SideBus

namespace LeftBus {
  namespace I2C {
    extern const uint32_t Frequency;
    enum : uint8_t {
      Bus = 0,
      SDA = 21,
      SCL = 22
    };
  };
  namespace LightSensor {
    extern const LightSensorConfig config;
  };
  namespace DistanceSensor {
    extern const DistanceSensorConfig config;
  };
  namespace Display {
    extern const DisplayConfig config;
  };
}
namespace RightBus {
  namespace I2C {
    extern const uint32_t Frequency;
    enum : uint8_t {
      Bus = 1,
      SDA = 32,
      SCL = 33
    };
  };
  namespace LightSensor {
    extern const LightSensorConfig config;
  };
  namespace DistanceSensor {
    extern const DistanceSensorConfig config;
  };
  namespace Display {
    extern const DisplayConfig config;
  };
}
namespace CommonBus {
  namespace Serial {
    extern const uint32_t Baudrate;
  };
  namespace SPI {
    extern const uint32_t Frequency;
    extern const int8_t MOSI;
    extern const int8_t MISO;
    extern const int8_t SCK;
  };
}
