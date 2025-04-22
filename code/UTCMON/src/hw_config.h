#pragma once
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include "UI.h"

namespace LeftBus {
  namespace I2C {
    uint32_t Frequency = 100000;
    enum : uint8_t {
      Bus = 0,
      SDA = 21,
      SCL = 22
    };
  };
  namespace LightSensor {
    uint8_t I2CAddress = 0x39;
  };
  namespace DistanceSensor {
    bool DebugEnabled = false;
    uint8_t I2CAddress = 0x29;
    Adafruit_VL53L0X::VL53L0X_Sense_config_t Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED;
  };
  namespace Display {
    DisplayConfig config {
      .Frequency = 15000000,
      .CS = 5,
      .DC = 0,
      .RESET = 4
    };
  };
}
namespace RightBus {
  namespace I2C {
    uint32_t Frequency = 100000;
    enum : uint8_t {
      Bus = 1,
      SDA = 32,
      SCL = 33
    };
  };
  namespace LightSensor {
    uint8_t I2CAddress = 0x39;
  };
  namespace DistanceSensor {
    bool DebugEnabled = false;
    uint8_t I2CAddress = 0x29;
    Adafruit_VL53L0X::VL53L0X_Sense_config_t Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED;
  };
  namespace Display {
    DisplayConfig config {
      .Frequency = 15000000,
      .CS = 25,
      .DC = 26,
      .RESET = 27
    };
  };
}
namespace CommonBus {
  namespace Serial {
    uint32_t Baudrate = 115200;
  };
  namespace SPI {
    uint32_t Frequency = 15000000;
  };
}
