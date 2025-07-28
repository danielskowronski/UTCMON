#pragma once
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include "UI.h"
#include "LightSensors.h"
#include "DistanceSensors.h"

#define kHz 1000
#define MHz 1000000
#define SPI_FREQUENCY 30*MHz

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
  namespace Network {
    extern const int ConnCheckPeriodMs;
    extern const int ConnCheckCount;
  }
  namespace Logging {
    extern const uint8_t Level;
  }
  namespace Loops {
    extern const uint32_t DisplayTaskPeriodMs;
    extern const uint32_t LoopTaskPeriodMs;
  }
}