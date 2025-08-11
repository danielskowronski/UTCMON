#include "debug.h"
#include "Logging.h"
#include "sys_config.h"
#include "hw_config.h"

void loopCheckPinMux() {
  for (size_t i = 0; i < sizeof(PinMuxTable)/sizeof(PinMuxTable[0]); ++i) {
    uint32_t cur = REG_READ(PinMuxTable[i].reg);
    if (cur != PinMuxInitialVals[i]) {
      logger.error(TAG_SYS, 
        "IOMUX change on %s: 0x%08X → %08X",
        PinMuxTable[i].name, PinMuxInitialVals[i], cur
      );
      gpio_dump_io_configuration(stdout, (1ULL << i));
      PinMuxInitialVals[i] = cur;
    }
  }
}
void setupCheckPinMux(){
  for (size_t i = 0; i < sizeof(PinMuxTable)/sizeof(PinMuxTable[0]); ++i) {
    PinMuxInitialVals[i] = REG_READ(PinMuxTable[i].reg);
    //logger.info(TAG_SYS, "Pin %s initial value: 0x%08X", PinMuxTable[i].name, PinMuxInitialVals[i]);
  }
  gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
}

void reconfigureSpiDrive(gpio_drive_cap_t strength) {
  // TODO: PARAMETRIZE WHICH PINS TO RECONFIGURE
  gpio_set_drive_capability((gpio_num_t)CommonBus::SPI::SCK,  strength);
  gpio_set_drive_capability((gpio_num_t)CommonBus::SPI::MOSI, strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.CS,    strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.DC,    strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.RESET, strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.CS,    strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.DC,    strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.RESET, strength);
}

void rotateWiFiPsMode() {
  wifi_ps_type_t ps;
  esp_wifi_get_ps(&ps);
  //logger.info(TAG_SYS, "Station PS mode = %s", (ps==WIFI_PS_NONE) ? "WIFI_PS_NONE" : (ps==WIFI_PS_MIN_MODEM) ? "WIFI_PS_MIN_MODEM" : "WIFI_PS_MAX_MODEM");
  wifi_ps_mode= (wifi_ps_mode+1)%3;
  esp_wifi_set_ps(wifi_ps_type_t(wifi_ps_mode));
}

void startRmtClobber(uint8_t pin) {
  rmt_config_t cfg = {
    .rmt_mode      = RMT_MODE_TX,
    .channel       = RMT_CHANNEL_0,
    .gpio_num      = gpio_num_t(pin),
    .clk_div       = 40,
    .mem_block_num = 1,
  };
  rmt_config(&cfg);
  rmt_driver_install(cfg.channel, 0, 0);
  rmt_item32_t item = {{{1, 1, 1, 0}}};
  rmt_write_items(cfg.channel, &item, 1, true);
}