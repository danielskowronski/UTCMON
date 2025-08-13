#pragma once

#ifndef U8G2_16BIT
#define U8G2_16BIT
#endif

#include <Arduino.h>
#include <U8g2lib.h>
#include <DateTime.h>
#include <SPI.h>
#include "common.h"
#include "DistanceSensor.h"
#include "DistanceSensors.h"
#include "hw_config.h"

#define DISPLAY_SENDBUFFER_DURATION_WARN_US 20000 // TODO: calculate this value
#define DISPLAY_SENDBUFFER_JITTER_WARN_US 1000    // 1000

#define DISP_W 256
#define DISP_H 64

#define UIL_ICN_FF u8g2_font_streamline_all_t // https://github.com/olikraus/u8g2/wiki/fntgrpstreamline#streamline_all
#define UIL_ICN_FW 20                         // spec is 21, but 20 is fine
#define UIL_ICN_FH 20                         // spec is 21, but 20 is fine
#define UIL_ICN_X 0
#define UIL_ICN_Y UIL_ICN_FH
#define UIL_ICN_W UIL_ICN_FW * 1     // 1 icon
#define UIL_ICN_GLYPH_OFFLINE 0x0206 // 0x019e
#define UIL_ICN_GLYPH_ONLINE 0x01fd

#define UIL_TOP_FF u8g2_font_logisoso16_tr // https://github.com/olikraus/u8g2/wiki/fntgrplogisoso#logisoso16
#define UIL_TOP_FH UIL_ICN_FH
#define UIL_TOP_FW 15
#define UIL_TOP_X UIL_ICN_W
#define UIL_TOP_Y UIL_TOP_FH

#define UIL_DAT_FF u8g2_font_logisoso38_tn // https://github.com/olikraus/u8g2/wiki/fntgrplogisoso#logisoso38
#define UIL_DAT_FW (22 + 2)                // spec is 22, char dist is 2
#define UIL_DAT_FH 38
#define UIL_DAT_X 0
#define UIL_DAT_Y DISP_H - 1
#define UIL_DAT_W UIL_DAT_FW * 10 // "YYYY-MM-DD" is 10 characters long

// TODO: implement UIL_XTR_* for using spare space between end of date and end of screen (width: 16px, height: 38px)

#define UIL_INF_FF u8g2_font_t0_12_tr // https://github.com/olikraus/u8g2/wiki/fntgrpttyp0#t0_12
#define UIL_INF_FH 10
#define UIL_INF_FW 6

#define UIL_SEN_COL_WIDTH UIL_INF_FW * 7 // "1234mm!" / "12345lx"
#define UIL_SEN_COL_MARGIN 1
#define UIL_SEN_COL_PADDING 1
#define UIL_SEN_Y0 0
#define UIL_SEN_X2 DISP_W - UIL_SEN_COL_WIDTH // was: 215
#define UIL_SEN_X1 UIL_SEN_X2 - UIL_SEN_COL_PADDING - UIL_SEN_COL_MARGIN - UIL_SEN_COL_PADDING - UIL_SEN_COL_WIDTH
#define UIL_SEN_W DISP_W - UIL_SEN_X1
#define UIL_SEN_H 2 * UIL_INF_FH

#define UIL_NTP_W UIL_INF_FW * 13 // "drift 123ms" / "sync 123 ago"
#define UIL_NTP_H 2 * UIL_INF_FH
#define UIL_NTP_X DISP_W - UIL_NTP_W
#define UIL_NTP_Y 0

enum CommonDisplayMode
{
  Normal = 0,
  Hollow,
  Blank,
  Count, // Always keep this as the last element to represent the number of modes
};
enum DateDisplayMode
{
  FullOnlyDate,
  FullAndSensors,
  FullAndNTP
};

struct DisplayContentsDate
{
  CommonDisplayMode commonDisplayMode;

  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t week;
  String weekday;

  time_t lastNtpSync;
  uint64_t lastDriftMs;

  uint16_t netIcon;

  bool colorsInverted;
};
struct DisplayContentsTime
{
  CommonDisplayMode commonDisplayMode;

  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  String timezone;

  bool colorsInverted;
};
#include "debug.h"
#ifdef UTCMON_SSD1322_PATCH
extern "C" void u8x8_ssd1322_set_inject(uint8_t enable, uint8_t at_byte, const uint8_t *bytes, uint8_t len, uint16_t ns);
extern "C" void u8x8_ssd1322_inject_cfg(uint8_t enable, uint16_t permille, uint8_t scope, uint8_t once, uint32_t seed);
extern "C" void u8x8_ssd1322_inject_enable(uint8_t enable);
extern "C" void u8x8_ssd1322_inject_rand_a0(uint8_t enable);
extern "C" void u8x8_ssd1322_inject_randa0_force_b(uint8_t enable, uint8_t bval);
extern "C" void u8x8_ssd1322_inject_rand_idx(uint8_t min_idx, uint8_t max_idx);
extern "C" void u8x8_ssd1322_inject_rand_ns(uint16_t min_ns, uint16_t max_ns);
#endif
class UI
{
private:
  DateDisplayMode dateDisplayMode = FullOnlyDate;
  CommonDisplayMode commonDisplayMode = Normal;
  U8G2 left;
  U8G2 right;
  DisplayConfig leftConfig;
  DisplayConfig rightConfig;
  void resetOneScreen(U8G2 &screen);
  void sendBuffer(U8G2 &screen);
  uint32_t drawClockDate(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r, uint16_t netIcon);
  uint32_t drawClockTime(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r);
  uint32_t drawBlank(U8G2 &screen);
  DisplayContentsDate displayContentsDate;
  DisplayContentsTime displayContentsTime;
  bool needToRedrawDate(DisplayContentsDate newDisplayContentsDate);
  bool needToRedrawTime(DisplayContentsTime newDisplayContentsTime);
  void setInvert(U8G2 &screen, bool enable);
  void setFont(U8G2 &screen, u8_t size);
  void setFont(U8G2 &screen, u8_t size, bool outline);
  // debug
  void printHelpOnce();
  bool sendCmdWithArgs_Left(uint8_t cmd, const uint8_t *args, int argc);
  void drawPatternBoth();
  void drawTilePattern(U8G2 &d);

public:
  UI(DisplayConfig leftConfig, DisplayConfig rightConfig);
  DevicePairInitSuccess init();
  void resetScreens();
  void drawSplashScreen(String version);
  void drawInitScreenSensor(String version, bool leftDistanceSensor, bool leftLightSensor, bool rightDistanceSensor, bool rightLightSensor);
  void drawInitScreenNetPhase1(String ssid);
  void drawInitScreenNetPhase2(String ipAddress);
  void drawInitScreenNetPhase3();
  void drawInitScreenNetPhase4(int driftMs);
  void setDateDisplayMode(DateDisplayMode mode);
  void setCommonDisplayMode(CommonDisplayMode mode);
  CommonDisplayMode getCommonDisplayMode();
  void drawClock(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r);
  void setContrast(int contrast);
  static String commonDisplayModeName(CommonDisplayMode mode)
  {
    switch (mode)
    {
    case Normal:
      return "Normal";
    case Hollow:
      return "Hollow";
    case Blank:
      return "Blank";
    default:
      return "Unknown";
    }
  }
  // debug
  void debugLoop();
};
