#pragma once
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include "UI.h"
#include "LightSensors.h"
#include "DistanceSensors.h"

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
    LightSensorConfig config {
      .I2CAddress = 0x39,
      .DebugEnabled = false,
      .AutoRangeEnabled = true,
      .IntegrationTime = TSL2561_INTEGRATIONTIME_13MS,
      .Gain = TSL2561_GAIN_16X
    };
  };
  namespace DistanceSensor {
    DistanceSensorConfig config {
      .I2CAddress = 0x29,
      .DebugEnabled = false,
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED
    };
  };
  namespace Display {
    DisplayConfig config {
      .Frequency = 3000000, //15000000,
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
    LightSensorConfig config {
      .I2CAddress = 0x39,
      .DebugEnabled = false,
      .AutoRangeEnabled = true,
      .IntegrationTime = TSL2561_INTEGRATIONTIME_13MS,
      .Gain = TSL2561_GAIN_16X
    };
    uint8_t I2CAddress = 0x39;
  };
  namespace DistanceSensor {
    DistanceSensorConfig config {
      .I2CAddress = 0x29,
      .DebugEnabled = false,
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED
    };
  };
  namespace Display {
    DisplayConfig config {
      .Frequency = 3000000, //15000000,
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
    uint32_t Frequency = 3000000; //15000000;
    int8_t MOSI = 23;
    int8_t MISO = 19;
    int8_t SCK = 18;
  };
}

namespace System {
  namespace PeriodicDisplayReset {
    uint64_t Period = 1*60*60; // 1h
    uint64_t LastReset;
  }
}