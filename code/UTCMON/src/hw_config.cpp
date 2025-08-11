#include "hw_config.h"

namespace LeftBus {
  namespace I2C {
    const uint32_t Frequency = 100*kHz;
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
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED,
      .TriggeringHoldUs = 150*1000, // 150ms
      .TriggeringThreshold = 100, // 10cm
      .TriggeringIgnore = 20, // 2cm
      .alias = "LEFT "
    };
  }
  namespace Display {
    const DisplayConfig config = {
      .Frequency = SPI_FREQUENCY,
      .CS = 5,
      .DC = 0,
      .RESET = 4
    };
  }
}

namespace RightBus {
  namespace I2C {
    const uint32_t Frequency = 100*kHz;
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
      .Mode = Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED,
      .TriggeringHoldUs = 150*1000, // 150ms
      .TriggeringThreshold = 100, // 10cm
      .TriggeringIgnore = 20, // 2cm
      .alias = "RIGHT"
    };
  }
  namespace Display {
    const DisplayConfig config = {
      .Frequency = SPI_FREQUENCY,
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
    const uint32_t Frequency = SPI_FREQUENCY;
    const int8_t MOSI = 23; // VSPI MOSI pin
    const int8_t MISO = 19; // VSPI MISO pin
    const int8_t SCK = 18;  // VSPI SCLK pin
  }
}
