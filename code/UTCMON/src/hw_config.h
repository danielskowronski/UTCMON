#pragma once
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include "UI.h"
#include "LightSensors.h"
#include "DistanceSensors.h"

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

namespace System {
  namespace PeriodicDisplayReset {
    extern const uint64_t Period;
  }
  namespace NTP {
    extern const uint64_t SyncIntervalMs;
    extern const uint64_t CheckPeriodS;
    extern const char* ServerHost;
    extern const int ServerPort;
    extern const int LocalPort;
  }
  namespace VirtualButtons {
    extern const uint32_t TimeToActivateMs;
  }
  namespace Network {
    extern const int ConnCheckPeriodMs;
    extern const int ConnCheckCount;
  }
  namespace Logging {
    extern const uint8_t Level;
  }
}