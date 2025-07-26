#pragma once

#ifndef U8G2_16BIT
#define U8G2_16BIT
#endif

#include <Arduino.h>
#include <U8g2lib.h>
#include <DateTime.h>
#include <SPI.h>
#include "common.h"
#include "DistanceSensors.h"

struct DisplayConfig {
  uint32_t Frequency;
  uint8_t CS;
  uint8_t DC;
  uint8_t RESET;
};

enum DateDisplayMode {
  FullOnlyDate,
  FullAndSensors,
  FullAndNTP
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
