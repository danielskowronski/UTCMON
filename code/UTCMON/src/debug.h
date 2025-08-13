#pragma once

/* STRESS_TEST_* configurable in platformio.ini

- STRESS_TEST_LOOP:    increase tasks loop frequency
- STRESS_TEST_WIFI:    rotate WiFi power save modes
- STRESS_TEST_CLOBBER: start RMT clobbering on SPI pins
- STRESS_TEST_DRAW:    always redraws screens
- STRESS_TEST_U8G2:    various debugging features for u8g2

*/

/*
STRESS_TEST_U8G2:

ginj a0 06 11 0 0
ginj prob 100 tile
ginj randa0 11
*/

#include <Arduino.h>
#include "soc/io_mux_reg.h"
#include "soc/rtc_io_reg.h"
#include "esp_wifi.h"
#include <driver/rmt.h>
#include <esp32-hal-ledc.h>
#include "driver/ledc.h"

struct PinMux
{
  const char *name;
  uint32_t reg;
};
static PinMux PinMuxTable[] = {
    {"GPIO00", PERIPHS_IO_MUX_GPIO0_U},
    {"GPIO01", PERIPHS_IO_MUX_U0TXD_U},
    {"GPIO02", PERIPHS_IO_MUX_GPIO2_U},
    {"GPIO03", PERIPHS_IO_MUX_U0RXD_U},
    {"GPIO04", PERIPHS_IO_MUX_GPIO4_U},
    {"GPIO05", PERIPHS_IO_MUX_GPIO5_U},
    {"GPIO06", PERIPHS_IO_MUX_SD_CLK_U},
    {"GPIO07", PERIPHS_IO_MUX_SD_DATA0_U},
    {"GPIO08", PERIPHS_IO_MUX_SD_DATA1_U},
    {"GPIO09", PERIPHS_IO_MUX_SD_DATA2_U},
    {"GPIO10", PERIPHS_IO_MUX_SD_DATA3_U},
    {"GPIO11", PERIPHS_IO_MUX_SD_CMD_U},
    {"GPIO12", PERIPHS_IO_MUX_MTDI_U},
    {"GPIO13", PERIPHS_IO_MUX_MTCK_U},
    {"GPIO14", PERIPHS_IO_MUX_MTMS_U},
    {"GPIO15", PERIPHS_IO_MUX_MTDO_U},
    {"GPIO16", PERIPHS_IO_MUX_GPIO16_U},
    {"GPIO17", PERIPHS_IO_MUX_GPIO17_U},
    {"GPIO18", PERIPHS_IO_MUX_GPIO18_U},
    {"GPIO19", PERIPHS_IO_MUX_GPIO19_U},
    {"GPIO20", PERIPHS_IO_MUX_GPIO20_U},
    {"GPIO21", PERIPHS_IO_MUX_GPIO21_U},
    {"GPIO22", PERIPHS_IO_MUX_GPIO22_U},
    {"GPIO23", PERIPHS_IO_MUX_GPIO23_U},
    {"GPIO24", PERIPHS_IO_MUX_GPIO24_U},
    {"GPIO25", PERIPHS_IO_MUX_GPIO25_U},
    {"GPIO26", PERIPHS_IO_MUX_GPIO26_U},
    {"GPIO27", PERIPHS_IO_MUX_GPIO27_U},
    {"GPIO32", PERIPHS_IO_MUX_GPIO32_U},
    {"GPIO33", PERIPHS_IO_MUX_GPIO33_U},
    {"GPIO34", PERIPHS_IO_MUX_GPIO34_U},
    {"GPIO35", PERIPHS_IO_MUX_GPIO35_U},
    {"GPIO36", PERIPHS_IO_MUX_GPIO36_U},
    {"GPIO37", PERIPHS_IO_MUX_GPIO37_U},
    {"GPIO38", PERIPHS_IO_MUX_GPIO38_U},
    {"GPIO39", PERIPHS_IO_MUX_GPIO39_U}};
static uint32_t PinMuxInitialVals[sizeof(PinMuxTable) / sizeof(PinMuxTable[0])];
void setupCheckPinMux();
void loopCheckPinMux();

void reconfigureSpiDrive(gpio_drive_cap_t strength);

static int wifi_ps_mode = 0;
void rotateWiFiPsMode();

void startRmtClobber(uint8_t pin = 5);

// used in UI

bool parseHexByte(const String &tok, uint8_t &out);
int parseHexLine(const String &line, uint8_t *out, int maxOut);
void printHex2(uint8_t v);
String readLine();
