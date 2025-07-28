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

#define DISPLAY_SENDBUFFER_DURATION_WARN_US 8000
#define DISPLAY_SENDBUFFER_JITTER_WARN_US   1000

#define DISP_W 256
#define DISP_H 64

#define UIL_ICN_FF u8g2_font_streamline_all_t // https://github.com/olikraus/u8g2/wiki/fntgrpstreamline#streamline_all
#define UIL_ICN_FW 20 // spec is 21, but 20 is fine
#define UIL_ICN_FH 20 // spec is 21, but 20 is fine
#define UIL_ICN_X  0
#define UIL_ICN_Y  UIL_ICN_FH
#define UIL_ICN_W  UIL_ICN_FW*1 // 1 icon
#define UIL_ICN_GLYPH_OFFLINE 0x0206 // 0x019e
#define UIL_ICN_GLYPH_ONLINE  0x01fd

#define UIL_TOP_FF u8g2_font_logisoso16_tr // https://github.com/olikraus/u8g2/wiki/fntgrplogisoso#logisoso16
#define UIL_TOP_FH UIL_ICN_FH
#define UIL_TOP_FW 15
#define UIL_TOP_X  UIL_ICN_W
#define UIL_TOP_Y  UIL_TOP_FH

#define UIL_DAT_FF u8g2_font_logisoso38_tn // https://github.com/olikraus/u8g2/wiki/fntgrplogisoso#logisoso38
#define UIL_DAT_FW (22+2) // spec is 22, char dist is 2
#define UIL_DAT_FH 38
#define UIL_DAT_X  0
#define UIL_DAT_Y  DISP_H
#define UIL_DAT_W  UIL_DAT_FW*10 // "YYYY-MM-DD" is 10 characters long

// TODO: implement UIL_XTR_* for using spare space between end of date and end of screen (width: 16px, height: 38px)

#define UIL_INF_FF u8g2_font_t0_12_tr // https://github.com/olikraus/u8g2/wiki/fntgrpttyp0#t0_12
#define UIL_INF_FH 10
#define UIL_INF_FW 6

#define UIL_SEN_COL_WIDTH UIL_INF_FW*7 // "1234mm!" / "12345lx"
#define UIL_SEN_COL_MARGIN 1
#define UIL_SEN_COL_PADDING 1
#define UIL_SEN_Y0 0
#define UIL_SEN_X2 DISP_W-UIL_SEN_COL_WIDTH //was: 215
#define UIL_SEN_X1 UIL_SEN_X2-UIL_SEN_COL_PADDING-UIL_SEN_COL_MARGIN-UIL_SEN_COL_PADDING-UIL_SEN_COL_WIDTH
#define UIL_SEN_W  DISP_W-UIL_SEN_X1
#define UIL_SEN_H  2*UIL_INF_FH

#define UIL_NTP_W  UIL_INF_FW*13 // "drift 123ms" / "sync 123 ago"
#define UIL_NTP_H  2*UIL_INF_FH
#define UIL_NTP_X  DISP_W-UIL_NTP_W
#define UIL_NTP_Y  0



enum DateDisplayMode {
  FullOnlyDate,
  FullAndSensors,
  FullAndNTP
};

struct DisplayContentsDate {
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
struct DisplayContentsTime {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  String timezone;

  bool colorsInverted;
};

class UI
{
private:
  DateDisplayMode dateDisplayMode = FullOnlyDate;
  U8G2 left;
  U8G2 right;
  DisplayConfig leftConfig;
  DisplayConfig rightConfig;
  void resetOneScreen(U8G2 &screen);
  void sendBuffer(U8G2 &screen);
  uint32_t drawClockDate(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r, uint16_t netIcon);
  uint32_t drawClockTime(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r);
  DisplayContentsDate displayContentsDate;
  DisplayContentsTime displayContentsTime;
  bool needToRedrawDate(DisplayContentsDate newDisplayContentsDate);
  bool needToRedrawTime(DisplayContentsTime newDisplayContentsTime);
  void setInvert(U8G2 &screen, bool enable);
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
  void drawClock(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r);
  void setContrast(int contrast);
};
