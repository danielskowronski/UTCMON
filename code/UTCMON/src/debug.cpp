#include <Arduino.h>
#include "debug.h"
#include "Logging.h"
#include "sys_config.h"
#include "hw_config.h"

void loopCheckPinMux()
{
  for (size_t i = 0; i < sizeof(PinMuxTable) / sizeof(PinMuxTable[0]); ++i)
  {
    uint32_t cur = REG_READ(PinMuxTable[i].reg);
    if (cur != PinMuxInitialVals[i])
    {
      logger.error(TAG_SYS,
                   "IOMUX change on %s: 0x%08X → %08X",
                   PinMuxTable[i].name, PinMuxInitialVals[i], cur);
      gpio_dump_io_configuration(stdout, (1ULL << i));
      PinMuxInitialVals[i] = cur;
    }
  }
}
void setupCheckPinMux()
{
  for (size_t i = 0; i < sizeof(PinMuxTable) / sizeof(PinMuxTable[0]); ++i)
  {
    PinMuxInitialVals[i] = REG_READ(PinMuxTable[i].reg);
    // logger.info(TAG_SYS, "Pin %s initial value: 0x%08X", PinMuxTable[i].name, PinMuxInitialVals[i]);
  }
  gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
}

void reconfigureSpiDrive(gpio_drive_cap_t strength)
{
  // TODO: PARAMETRIZE WHICH PINS TO RECONFIGURE
  gpio_set_drive_capability((gpio_num_t)CommonBus::SPI::SCK, strength);
  gpio_set_drive_capability((gpio_num_t)CommonBus::SPI::MOSI, strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.CS, strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.DC, strength);
  gpio_set_drive_capability((gpio_num_t)LeftBus::Display::config.RESET, strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.CS, strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.DC, strength);
  gpio_set_drive_capability((gpio_num_t)RightBus::Display::config.RESET, strength);
}

void rotateWiFiPsMode()
{
  wifi_ps_type_t ps;
  esp_wifi_get_ps(&ps);
  // logger.info(TAG_SYS, "Station PS mode = %s", (ps==WIFI_PS_NONE) ? "WIFI_PS_NONE" : (ps==WIFI_PS_MIN_MODEM) ? "WIFI_PS_MIN_MODEM" : "WIFI_PS_MAX_MODEM");
  wifi_ps_mode = (wifi_ps_mode + 1) % 3;
  esp_wifi_set_ps(wifi_ps_type_t(wifi_ps_mode));
}

void startRmtClobber(uint8_t pin)
{
  rmt_config_t cfg = {
      .rmt_mode = RMT_MODE_TX,
      .channel = RMT_CHANNEL_0,
      .gpio_num = gpio_num_t(pin),
      .clk_div = 40,
      .mem_block_num = 1,
  };
  rmt_config(&cfg);
  rmt_driver_install(cfg.channel, 0, 0);
  rmt_item32_t item = {{{1, 1, 1, 0}}};
  rmt_write_items(cfg.channel, &item, 1, true);
}

// used in UI
// WARN BELOW: temporary machine-generated code

static inline bool isSpaceChar(char c)
{
  return c == ' ' || c == '\t';
}

bool parseHexByte(const String &tok, uint8_t &out)
{
  int start = 0;
  if (tok.length() >= 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X'))
    start = 2;
  int n = tok.length() - start;
  if (n <= 0 || n > 2)
    return false;
  uint8_t val = 0;
  for (int i = start; i < tok.length(); ++i)
  {
    char c = tok[i];
    uint8_t nib;
    if (c >= '0' && c <= '9')
      nib = c - '0';
    else if (c >= 'a' && c <= 'f')
      nib = 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F')
      nib = 10 + (c - 'A');
    else
      return false;
    val = (uint8_t)((val << 4) | nib);
  }
  out = val;
  return true;
}

int parseHexLine(const String &line, uint8_t *out, int maxOut)
{
  int count = 0;
  int i = 0;
  while (i < line.length())
  {
    while (i < line.length() && isSpaceChar(line[i]))
      i++;
    if (i >= line.length())
      break;
    int start = i;
    while (i < line.length() && !isSpaceChar(line[i]))
      i++;
    String tok = line.substring(start, i);
    uint8_t b;
    if (!parseHexByte(tok, b))
      return -1;
    if (count >= maxOut)
      return -1;
    out[count++] = b;
  }
  return count;
}

void printHex2(uint8_t v)
{
  const char *hexd = "0123456789ABCDEF";
  Serial.print(hexd[(v >> 4) & 0xF]);
  Serial.print(hexd[v & 0xF]);
}

String readLine()
{
  String s;
  while (true)
  {
    while (Serial.available())
    {
      char c = (char)Serial.read();
      if (c == '\r')
        continue;
      if (c == '\n')
        return s;
      s += c;
      if (s.length() > 120)
      {
        while (Serial.available() && Serial.read() != '\n')
          ;
        return s;
      }
    }
    delay(1);
    if (s.length() == 0)
      Serial.flush();
  }
}