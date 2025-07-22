#include "hw_config.h"

namespace LeftBus {
  namespace I2C {
    const uint32_t Frequency = 100000; // 100 kHz
  }
  namespace LightSensor {
    const LightSensorConfig config = {
      .I2CAddress = 0x39,
      .DebugEnabled = false,
      .AutoRangeEnabled = true,
      .IntegrationTime = TSL2561_INTEGRATIONTIME_13MS,
      .Gain = TSL2561_GAIN_16X
    };
  }
  namespace DistanceSensor {
    const DistanceSensorConfig config = {
      .I2CAddress = 0x29,
      .DebugEnabled = false,
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED
    };
  }
  namespace Display {
    const DisplayConfig config = {
      .Frequency = 3000000, // 300 kHz
      .CS = 5,
      .DC = 0,
      .RESET = 4
    };
  }
}

namespace RightBus {
  namespace I2C {
    const uint32_t Frequency = 100000; // 100 kHz
  }
  namespace LightSensor {
    const LightSensorConfig config = {
      .I2CAddress = 0x39,
      .DebugEnabled = false,
      .AutoRangeEnabled = true,
      .IntegrationTime = TSL2561_INTEGRATIONTIME_13MS,
      .Gain = TSL2561_GAIN_16X
    };
  }
  namespace DistanceSensor {
    const DistanceSensorConfig config = {
      .I2CAddress = 0x29,
      .DebugEnabled = false,
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED
    };
  }
  namespace Display {
    const DisplayConfig config = {
      .Frequency = 3000000, // 300 kHz
      .CS = 25,
      .DC = 26,
      .RESET = 27
    };
  }
}

namespace CommonBus {
  namespace Serial {
    const uint32_t Baudrate = 115200;
  }
  namespace SPI {
    const uint32_t Frequency = 3000000; // 300 kHz, 1.5MHz is too fast for the display
    const int8_t MOSI = 23; // VSPI MOSI pin
    const int8_t MISO = 19; // VSPI MISO pin
    const int8_t SCK = 18;  // VSPI SCLK pin
  }
}

namespace System {
  namespace PeriodicDisplayReset {
    const uint64_t Period = 1*60*60; // 1 hour
  }
  namespace NTP {
    const uint64_t SyncIntervalMs = 15*60*1000; // 15 minutes
    const uint64_t CheckPeriodS = 60; // 1 minute
    const char* ServerHost = "pool.ntp.org";
    const int ServerPort = 123;
    const int LocalPort = 2390; // arbitrary, should not conflict with other services
  }
  namespace VirtualButtons {
    const uint32_t TimeToActivateMs = 500;
  }
  namespace Network {
    extern const int ConnCheckPeriodMs = 500;
    extern const int ConnCheckCount = 60*(1000 / ConnCheckPeriodMs); // 1 minute
  }
  namespace Logging {
    // WARN: ARDUHAL_LOG_LEVEL_INFO < ARDUHAL_LOG_LEVEL_DEBUG < ARDUHAL_LOG_LEVEL_VERBOSE
    extern const uint8_t Level = ARDUHAL_LOG_LEVEL_VERBOSE; // ARDUHAL_LOG_LEVEL_INFO / ARDUHAL_LOG_LEVEL_DEBUG
  }
}