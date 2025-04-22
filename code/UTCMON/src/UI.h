#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <DateTime.h>

struct DisplayConfig {
  uint32_t Frequency;
  uint8_t CS;
  uint8_t DC;
  uint8_t RESET;
};


class UI
{
private:
  U8G2 left;
  U8G2 right;
public:
  UI(DisplayConfig leftConfig, DisplayConfig rightConfig);
  void drawSplashScreen(String version);
  void drawInitScreenSensor(String version, bool leftDistanceSensor, bool leftLightSensor, bool rightDistanceSensor, bool rightLightSensor);
  void drawInitScreenNetPhase1(String ssid);
  void drawInitScreenNetPhase2();
  void drawInitScreenNetPhase3();
  void drawInitScreenNetPhase4();
  void drawClock(DateTimeStruct dt, int mm_l, int mm_r, int lux_l, int lux_r);
  void setContrast(int contrast);
};
