#include "sys_config.h"

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
  namespace Network {
    extern const int ConnCheckPeriodMs = 500;
    extern const int ConnCheckCount = 60*(1000 / ConnCheckPeriodMs); // 1 minute
  }
  namespace Logging {
    // WARN: ARDUHAL_LOG_LEVEL_INFO < ARDUHAL_LOG_LEVEL_DEBUG < ARDUHAL_LOG_LEVEL_VERBOSE
    extern const uint8_t Level = ARDUHAL_LOG_LEVEL_DEBUG; // ARDUHAL_LOG_LEVEL_INFO / ARDUHAL_LOG_LEVEL_DEBUG
  }
  namespace Loops {
    const uint32_t DisplayTaskPeriodMs = 100;
    const uint32_t LoopTaskPeriodMs = 2000;
  }
  namespace DisplayModes {
    const char* const timezones[] = {
      "Europe/Warsaw",
      "UTC"
    };
    const size_t timezones_count = sizeof(timezones) / sizeof(timezones[0]);
  }
}
void sys_config_overrides(){
  #ifdef STRESS_TEST_LOOP
  System::Loops::DisplayTaskPeriodMs = 1;
  System::Loops::LoopTaskPeriodMs = 100;
  #endif
}