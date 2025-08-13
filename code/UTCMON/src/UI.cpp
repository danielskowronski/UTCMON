#include "UI.h"
#include "common.h"
#include "TimeSync.h"
#include "Logging.h"
#include "WiFi.h"
#include <U8g2lib.h>
#include <new>

#include "Fonts.h"

void UI::setFont(U8G2 &screen, u8_t size)
{
  // FIXME: ensure all setFonts are set in the same way, also support different font variants (text, graphic, different charset) + all those OIL_FF
  this->setFont(screen, size, this->commonDisplayMode == Hollow);
}
void UI::setFont(U8G2 &screen, u8_t size, bool outline)
{
  if (outline)
  {
    switch (size)
    {
    case 62:
      screen.setFont(u8g2_font_logisoso62_outline_tn);
      break; // u8g2_font_spleen32x64_mn
    // case 46: screen.setFont(u8g2_font_logisoso46_outline_tn); break;
    case 38:
      screen.setFont(u8g2_font_logisoso38_outline_tn);
      break;
    case 16:
      screen.setFont(u8g2_font_calibration_gothic_nbp_tr);
      break;
    default:
      screen.setFont(u8g2_font_logisoso16_tr);
      break; // fallback
    }
  }
  else
  {
    switch (size)
    {
    case 62:
      screen.setFont(u8g2_font_logisoso62_orig_tn);
      break;
    case 46:
      screen.setFont(u8g2_font_logisoso46_tn);
      break;
    case 38:
      screen.setFont(u8g2_font_logisoso38_orig_tn);
      break;
    case 16:
      screen.setFont(u8g2_font_logisoso16_tr);
      break;
    default:
      screen.setFont(u8g2_font_logisoso16_tr);
      break; // fallback
    }
  }
}
UI::UI(DisplayConfig leftConfig, DisplayConfig rightConfig)
{
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
void UI::sendBuffer(U8G2 &screen)
{
  // noInterrupts();
  //  u8x8_d_ssd1322_common
  if (this->commonDisplayMode == Hollow)
  {
  }
  screen.sendBuffer();
  // interrupts();
}
DevicePairInitSuccess UI::init()
{
  DevicePairInitSuccess initSuccess;
  new (&this->left) U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, leftConfig.CS, leftConfig.DC, leftConfig.RESET);
  this->left.setBusClock(leftConfig.Frequency);
  initSuccess.left = this->left.begin();

  new (&this->right) U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, rightConfig.CS, rightConfig.DC, rightConfig.RESET);
  this->right.setBusClock(rightConfig.Frequency);
  initSuccess.right = this->right.begin();

  this->resetScreens();

  return initSuccess;
}
void UI::resetOneScreen(U8G2 &screen)
{
  screen.clearBuffer();
  this->sendBuffer(screen);

  delayMicroseconds(1 * 1000);

  screen.setDrawColor(1);
  screen.drawBox(0, 0, 256, 64);
  this->sendBuffer(screen);

  delayMicroseconds(1 * 1000);

  screen.clearBuffer();
  this->sendBuffer(screen);
}
void UI::resetScreens()
{
  this->resetOneScreen(this->left);
  this->resetOneScreen(this->right);
}
void UI::setDateDisplayMode(DateDisplayMode mode)
{
  this->dateDisplayMode = mode;
}
void UI::setCommonDisplayMode(CommonDisplayMode mode)
{
  this->commonDisplayMode = mode;
}
CommonDisplayMode UI::getCommonDisplayMode()
{
  return this->commonDisplayMode;
}
bool UI::needToRedrawDate(DisplayContentsDate newDisplayContentsDate)
{
#ifdef STRESS_TEST_DRAW
  return true;
#endif
  bool need = false;
  if (this->displayContentsTime.commonDisplayMode == Blank && newDisplayContentsDate.commonDisplayMode == Blank)
  {
    need = false;
  }
  else if (this->displayContentsDate.year != newDisplayContentsDate.year ||
           this->displayContentsDate.month != newDisplayContentsDate.month ||
           this->displayContentsDate.day != newDisplayContentsDate.day ||
           this->displayContentsDate.week != newDisplayContentsDate.week ||
           this->displayContentsDate.weekday != newDisplayContentsDate.weekday ||
           this->displayContentsDate.lastNtpSync != newDisplayContentsDate.lastNtpSync ||
           this->displayContentsDate.lastDriftMs != newDisplayContentsDate.lastDriftMs ||
           this->displayContentsDate.colorsInverted != newDisplayContentsDate.colorsInverted ||
           this->displayContentsDate.netIcon != newDisplayContentsDate.netIcon ||
           this->displayContentsDate.commonDisplayMode != newDisplayContentsDate.commonDisplayMode)
  {
    need = true;
  }
  return need;
}
void UI::setInvert(U8G2 &screen, bool enable)
{
  if (enable)
  {
    screen.setDrawColor(1);
    screen.drawBox(0, 0, DISP_W, DISP_H);
    screen.setDrawColor(0);
  }
  else
  {
    screen.setDrawColor(1);
  }
}
uint32_t UI::drawClockDate(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r, uint16_t netIcon)
{
  // FIXME: check if dt has valid date, if not - do not draw anything or draw "no date" message (issue happens on first draw after init)
  char buffer[256];
  uint32_t beforeFirst, afterFirst, deltaFirst;

  this->left.clearBuffer();
  this->setInvert(this->left, dsp.left.triggering);

  switch (this->dateDisplayMode)
  {
  case FullOnlyDate:
  case FullAndNTP:
  case FullAndSensors:
    // Left screen, bottom line, large: YYYY-MM-DD
    sprintf(buffer, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
    this->setFont(this->left, 38); // this->left.setFont(UIL_DAT_FF);
    this->left.drawStr(UIL_DAT_X, UIL_DAT_Y, buffer);
    // Left screen, top line, small: status icon ()
    this->left.setFont(UIL_ICN_FF);
    this->left.drawGlyph(UIL_ICN_X, UIL_ICN_Y, netIcon);

    // Left screen, top line, small: ISO week - Weekeday
    sprintf(buffer, "W%02d %.16s", dt.week, dt.weekday); // "Wednesday"
    this->setFont(this->left, 16);                       // this->left.setFont(UIL_TOP_FF);
    this->left.drawStr(UIL_TOP_X, UIL_TOP_Y, buffer);

    break;
  }

  if (this->commonDisplayMode != Hollow)
  {
    switch (this->dateDisplayMode)
    {
    case FullAndSensors:
      // Left screen, top line, small: distance
      this->left.setFont(UIL_INF_FF);
      this->left.drawStr(UIL_SEN_X1, UIL_INF_FH, DistanceSensor::fmtDist(dsp.left, true, true, true).c_str());
      sprintf(buffer, "%05dlx", lux_l);
      this->left.setFont(UIL_INF_FF);
      this->left.drawStr(UIL_SEN_X1, UIL_INF_FH * 2, buffer);

      this->left.drawBox(UIL_SEN_X2 - UIL_SEN_COL_PADDING, UIL_SEN_Y0, UIL_SEN_COL_MARGIN, UIL_SEN_H);

      this->left.setFont(UIL_INF_FF);
      this->left.drawStr(UIL_SEN_X2, UIL_INF_FH, DistanceSensor::fmtDist(dsp.right, true, true, true).c_str());
      sprintf(buffer, "%05dlx", lux_r);
      this->left.setFont(UIL_INF_FF);
      this->left.drawStr(UIL_SEN_X2, UIL_INF_FH * 2, buffer);
      break;
    case FullAndNTP:
      this->left.setFont(UIL_INF_FF);
      sprintf(buffer, "drift % 5dms", ntpDiagnostics.lastDriftMs);
      this->left.drawStr(UIL_NTP_X, UIL_INF_FH, buffer);
      sprintf(buffer, "sync % 3dm ago", minutesSinceLastNtpSync());
      this->left.drawStr(UIL_NTP_X, UIL_INF_FH * 2, buffer);
      break;
    default:
      // No additional info
      break;
    }
  }

  beforeFirst = micros();
  this->sendBuffer(this->left);
  afterFirst = micros();

  deltaFirst = afterFirst - beforeFirst;
  return deltaFirst;
}
uint32_t UI::drawClockTime(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r)
{
  char buffer[256];
  uint32_t beforeSecond, afterSecond, deltaSecond;

  this->right.clearBuffer();
  setInvert(this->right, dsp.right.triggering);

  // Right screen, bottom line, large: HH:MM
  /*
  sprintf(buffer, "%02d:%02d", dt.hour, dt.minute);
  this->setFont(this->right, 62);
  this->right.drawStr(0,64,buffer);
  */
  sprintf(buffer, "%02d", dt.hour);
  this->setFont(this->right, 62);
  this->right.drawStr(0, 64 - 1, buffer);
  sprintf(buffer, ":");
  this->setFont(this->right, 62);
  this->right.drawStr(2 * 36 + 12, 64 - 1, buffer);
  sprintf(buffer, "%02d", dt.minute);
  this->setFont(this->right, 62);
  this->right.drawStr(3 * 36, 64 - 1, buffer);

  // Right screen, bottom line continued, medium: :SS
  sprintf(buffer, ":%02d", dt.second);
  this->setFont(this->right, 38); // 46
  this->right.drawStr(5 * 36 + (8), 64 - 1, buffer);

  // Right screen, top line, above :SS, small: timezone
  sprintf(buffer, "%.4s", dt.timezone);
  this->setFont(this->right, 16);
  this->right.drawStr(5 * 36 + 26 + 5 * (strlen(dt.timezone.c_str()) % 2), 16 + 4, buffer);

  beforeSecond = micros();
  this->sendBuffer(this->right);
  afterSecond = micros();

  deltaSecond = afterSecond - beforeSecond;
  return deltaSecond;
}
bool UI::needToRedrawTime(DisplayContentsTime newDisplayContentsTime)
{
#ifdef STRESS_TEST_DRAW
  return true;
#endif
  bool need = false;
  if (this->displayContentsTime.commonDisplayMode == Blank && newDisplayContentsTime.commonDisplayMode == Blank)
  {
    need = false;
  }
  else if (this->displayContentsTime.hour != newDisplayContentsTime.hour ||
           this->displayContentsTime.minute != newDisplayContentsTime.minute ||
           this->displayContentsTime.second != newDisplayContentsTime.second ||
           this->displayContentsTime.timezone != newDisplayContentsTime.timezone ||
           this->displayContentsTime.colorsInverted != newDisplayContentsTime.colorsInverted ||
           this->displayContentsTime.commonDisplayMode != newDisplayContentsTime.commonDisplayMode)
  {
    need = true;
  }
  return need;
}
uint32_t UI::drawBlank(U8G2 &screen)
{
  uint32_t before, after, delta;
  screen.clearBuffer();
  screen.setDrawColor(0);
  screen.drawBox(0, 0, DISP_W, DISP_H);
  screen.setDrawColor(1);

  /*
  screen.drawPixel(0,0);
  screen.drawPixel(0,DISP_H-1);
  screen.drawPixel(DISP_W-1,DISP_H-1);
  screen.drawPixel(DISP_W-1,0);
  */

  before = micros();
  this->sendBuffer(screen);
  after = micros();

  delta = after - before;
  return delta;
}
void UI::drawClock(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r)
{
  char buffer[256];
  uint32_t beforeFirst, afterFirst, beforeSecond, afterSecond, deltaFirst = 0, deltaSecond = 0, jitter;

  uint16_t netIcon = (WiFi.status() != WL_CONNECTED) ? UIL_ICN_GLYPH_OFFLINE : UIL_ICN_GLYPH_ONLINE;

  // TODO: support updating only segments that changed

  bool needToRedrawDate, needToRedrawTime;
  DisplayContentsDate newDisplayContentsDate = {
      .commonDisplayMode = this->commonDisplayMode,
      .year = dt.year,
      .month = dt.month,
      .day = dt.day,
      .week = dt.week,
      .weekday = dt.weekday,
      .lastNtpSync = ntpDiagnostics.lastNtpSync,
      .lastDriftMs = ntpDiagnostics.lastDriftMs,
      .netIcon = netIcon,
      .colorsInverted = dsp.left.triggering};
  DisplayContentsTime newDisplayContentsTime = {
      .commonDisplayMode = this->commonDisplayMode,
      .hour = dt.hour,
      .minute = dt.minute,
      .second = dt.second,
      .timezone = dt.timezone,
      .colorsInverted = dsp.right.triggering};

  switch (this->dateDisplayMode)
  {
  case FullOnlyDate:
  case FullAndNTP:
    needToRedrawDate = this->needToRedrawDate(newDisplayContentsDate);
    break;
  case FullAndSensors:
    needToRedrawDate = true; // sensors always change
    break;
  }
  this->displayContentsDate = newDisplayContentsDate;
  if (needToRedrawDate)
  {
    if (this->commonDisplayMode == Blank)
      deltaFirst = this->drawBlank(this->left);
    else
      deltaFirst = this->drawClockDate(dt, dsp, lux_l, lux_r, netIcon);
  }

  needToRedrawTime = this->needToRedrawTime(newDisplayContentsTime);
  this->displayContentsTime = newDisplayContentsTime;
  if (needToRedrawTime)
  {
    if (this->commonDisplayMode == Blank)
      deltaSecond = this->drawBlank(this->right);
    else
      deltaSecond = this->drawClockTime(dt, dsp, lux_l, lux_r);
  }

  jitter = abs((int)deltaFirst - (int)deltaSecond);
  bool abnoramJitter = (jitter > DISPLAY_SENDBUFFER_JITTER_WARN_US);
  bool unexpectedTimeFirst = (deltaFirst > DISPLAY_SENDBUFFER_DURATION_WARN_US);
  bool unexpectedTimeSecond = (deltaSecond > DISPLAY_SENDBUFFER_DURATION_WARN_US);
  if (deltaFirst == 0 || deltaSecond == 0)
  {
    abnoramJitter = false; // if skipping one of the displays, then jitter cannot be calculated
  }

  // if any of displays take more than DISPLAY_SENDBUFFER_DURATION_WARN_US to update or if the jitter is more than DISPLAY_SENDBUFFER_JITTER_WARN_US then log a warning, else log as verbose-debug
  int logLevel = (abnoramJitter || unexpectedTimeFirst || unexpectedTimeSecond) ? ARDUHAL_LOG_LEVEL_WARN : ARDUHAL_LOG_LEVEL_VERBOSE;
#ifndef STRESS_TEST_DRAW // with debug, actual display time will always be high
  logger.log(logLevel, TAG_DISPLAYS, "drawClock: left sendBuffer took %8ld us, right sendBuffer took %8ld us, jitter was %8ld us", deltaFirst, deltaSecond, jitter);
#endif
}
void UI::drawSplashScreen(String version)
{
  this->left.clearBuffer();
  this->left.drawBox(0, 0, 256, 64);
  this->left.setDrawColor(0);
  this->left.setFont(u8g2_font_logisoso32_tr);
  this->left.drawStr(0, 32, "UTCMON Left");
  this->left.setFont(u8g2_font_profont17_mr);
  this->left.drawStr(0, 60, version.c_str());
  this->left.setDrawColor(1);
  this->sendBuffer(this->left);

  this->right.clearBuffer();
  this->right.drawBox(0, 0, 256, 64);
  this->right.setDrawColor(0);
  this->right.setFont(u8g2_font_logisoso32_tr);
  this->right.drawStr(0, 32, "UTCMON Right");
  this->right.setFont(u8g2_font_profont17_mr);
  this->right.drawStr(0, 60, "Initializing................................");
  this->right.setDrawColor(1);
  this->sendBuffer(this->right);
}
void UI::drawInitScreenSensor(String version, bool leftDistanceSensor, bool leftLightSensor, bool rightDistanceSensor, bool rightLightSensor)
{
  // TODO: fix this monstrosity (ported from proof of concept)
  char buffer[256];

  this->left.clearBuffer();
  this->left.drawStr(0, 20, version.c_str());
  this->left.drawStr(0, 40, "Dist. sensors: ");
  if (leftDistanceSensor)
    this->left.drawStr(130, 40, "_OK_");
  else
    this->left.drawStr(130, 40, "FAIL");
  if (rightDistanceSensor)
    this->left.drawStr(180, 40, "_OK_");
  else
    this->left.drawStr(180, 40, "FAIL");
  this->left.drawStr(0, 60, "Light sensors: ");
  if (leftLightSensor)
    this->left.drawStr(130, 60, "_OK_");
  else
    this->left.drawStr(130, 60, "FAIL");
  if (rightDistanceSensor)
    this->left.drawStr(180, 60, "_OK_");
  else
    this->left.drawStr(180, 60, "FAIL");
  this->sendBuffer(this->left);
}
void UI::drawInitScreenNetPhase1(String ssid)
{
  // TODO: support retry/wait count display
  char buffer[64];
  sprintf(buffer, "SSID: %s", ssid.c_str());
  this->right.setFont(u8g2_font_profont17_mr);
  this->right.clearBuffer();
  this->right.drawStr(0, 20, buffer);
  this->right.drawStr(0, 40, "WiFi: connecting...");
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase2(String ipAddress)
{
  char buffer[64];
  sprintf(buffer, "WiFi: %s", ipAddress.c_str());
  this->right.drawStr(0, 40, buffer);
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase3()
{
  this->right.drawStr(0, 60, " NTP: initiliazing...");
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase4(int driftMs)
{
  char buffer[64];
  sprintf(buffer, " NTP: synced, drift %d ms", driftMs);
  this->right.drawStr(0, 60, buffer);
  this->sendBuffer(this->right);
}

void UI::setContrast(int contrast)
{
  this->left.setContrast(contrast);
  this->right.setContrast(contrast);
}

// debug
// WARN BELOW: temporary machine-generated code

void UI::printHelpOnce()
{
  Serial.println();
  Serial.println(F("U8G2 SPI diagnostics:"));
  Serial.println(F("  <empty line>  → draw 8x8-tile diagnostic pattern to BOTH displays"));
  Serial.println(F("  <hex bytes>   → send to LEFT via sendF as 'c' + 'a…' (e.g. 'A0 FF' → ca)"));
  Serial.println(F("  ?             → help"));
  Serial.println(F("  q             → quit loop, proceed to UTCMON"));
#ifdef UTCMON_SSD1322_PATCH
  Serial.println(F("  ginj off|a0 [A B idx ns]|raw <b0..b3> [idx] [ns] → inject command mid-tile"));
  Serial.println(F("  ginj prob <permille 0..1000> [tile|band] [once] [seed] → probabilistic injector"));
  Serial.println(F("  ginj on/off → enable/disable injector without changing bytes/index"));
  Serial.println(F("  ginj randa0 on|off|11|01 → randomize A0; optional B forced to 11 or 01"));
  Serial.println(F("  ginj rand idx <min> <max> | ns <min> <max> → randomize byte index or ns width"));
#endif
}

bool UI::sendCmdWithArgs_Left(uint8_t cmd, const uint8_t *args, int argc)
{
  switch (argc)
  {
  case 0:
    left.sendF("c", cmd);
    break;
  case 1:
    left.sendF("ca", cmd, args[0]);
    break;
  case 2:
    left.sendF("caa", cmd, args[0], args[1]);
    break;
  case 3:
    left.sendF("caaa", cmd, args[0], args[1], args[2]);
    break;
  case 4:
    left.sendF("caaaa", cmd, args[0], args[1], args[2], args[3]);
    break;
  case 5:
    left.sendF("caaaaa", cmd, args[0], args[1], args[2], args[3], args[4]);
    break;
  case 6:
    left.sendF("caaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5]);
    break;
  case 7:
    left.sendF("caaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
    break;
  case 8:
    left.sendF("caaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
    break;
  case 9:
    left.sendF("caaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);
    break;
  case 10:
    left.sendF("caaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
    break;
  case 11:
    left.sendF("caaaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
    break;
  case 12:
    left.sendF("caaaaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11]);
    break;
  case 13:
    left.sendF("caaaaaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12]);
    break;
  case 14:
    left.sendF("caaaaaaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13]);
    break;
  case 15:
    left.sendF("caaaaaaaaaaaaaaa", cmd, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14]);
    break;
  default:
    return false;
  }
  return true;
}
void UI::drawPatternBoth()
{
  drawTilePattern(left);
  left.sendBuffer();
  drawTilePattern(right);
  right.sendBuffer();
  Serial.println(F("Pattern drawn on BOTH displays."));
}

void UI::drawTilePattern(U8G2 &d)
{
  static const char seq[16] = {
      '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 0 // 0 => solid square
  };
  d.clearBuffer();
  d.setDrawColor(1);
  d.setFont(u8g2_font_5x8_mf);

  for (int ty = 0; ty < 8; ++ty)
  {
    for (int tx = 0; tx < 32; ++tx)
    {
      int x = tx * 8;
      int y = ty * 8;
      char c = seq[(ty * 32 + tx) & 0x0F];

      if (c == 0)
      {
        d.drawBox(x + 1, y + 1, 6, 6);
      }
      else
      {
        d.drawGlyph(x + 1, y + 7, (uint16_t)c);
      }
    }
  }
}

void UI::debugLoop()
{
  printHelpOnce();
  enum LoopMode
  {
    LOOP_NONE = 0,
    LOOP_STRESS
  };
  static LoopMode loopMode = LOOP_NONE;
  static uint16_t loopSends = 3;
  static uint8_t stressSide = 2;     // 0=left, 1=right, 2=both
  static uint32_t stressDelayUs = 0; // inter-frame delay

  while (true)
  {
    if (loopMode == LOOP_STRESS)
    {
      if (Serial.available())
      {
        String cmd = readLine();
        cmd.trim();
        if (cmd == "q" || cmd == "quit" || cmd == "exit")
        {
          loopMode = LOOP_NONE;
          Serial.println(F("Stress mode: exit"));
        }
        else if (cmd.startsWith("d "))
        {
          String rest = cmd.substring(2);
          rest.trim();
          long v = rest.toInt();
          if (v < 0)
            v = 0;
          if (v > 1000000)
            v = 1000000;
          stressDelayUs = (uint32_t)v;
          Serial.print(F("Stress delay set to "));
          Serial.print(stressDelayUs);
          Serial.println(F(" us"));
        }
        else if (cmd == "left" || cmd == "right" || cmd == "both")
        {
          stressSide = (cmd == "left") ? 0 : (cmd == "right") ? 1
                                                              : 2;
          Serial.print(F("Stress side="));
          Serial.println((stressSide == 0) ? F("left") : (stressSide == 1) ? F("right")
                                                                           : F("both"));
        }
        else
        {
          Serial.println(F("(q=quit, 'd <us>' delay, 'left'/'right'/'both')"));
        }
      }
      if (stressSide != 1)
        left.sendBuffer(); 
      if (stressSide != 0)
        right.sendBuffer(); 
      if (stressDelayUs)
        delayMicroseconds(stressDelayUs);
      continue; 
    }

    Serial.print(F("> "));
    String line = readLine(); 
    line.trim();

    if (line.length() == 0)
    {
      drawPatternBoth();
      continue;
    }
    if (line == "?" || line == "help")
    {
      printHelpOnce();
      continue;
    }
    if (line == "q" || line == "quit")
    {
      break;
    }

    if (line.startsWith("stress"))
    {
      // syntax: stress [both|left|right] [delay_us]
      String rest = line.substring(6);
      rest.trim();
      stressSide = 2;
      stressDelayUs = 0;
      if (rest.length())
      {
        int sp = rest.indexOf(' ');
        String t1 = (sp < 0) ? rest : rest.substring(0, sp);
        String t2 = (sp < 0) ? String() : rest.substring(sp + 1);
        t1.trim();
        t2.trim();
        if (t1.equalsIgnoreCase("left"))
          stressSide = 0;
        else if (t1.equalsIgnoreCase("right"))
          stressSide = 1;
        else if (t1.equalsIgnoreCase("both") || t1.length() == 0)
          stressSide = 2;
        else
        {
          long v = t1.toInt();
          if (v >= 0)
            stressDelayUs = (uint32_t)min<long>(v, 1000000);
        }
        if (t2.length())
        {
          long v = t2.toInt();
          if (v >= 0)
            stressDelayUs = (uint32_t)min<long>(v, 1000000);
        }
      }
      if (stressSide != 1)
      {
        drawTilePattern(left);
        left.sendBuffer();
      }
      if (stressSide != 0)
      {
        drawTilePattern(right);
        right.sendBuffer();
      }
      loopMode = LOOP_STRESS;
      Serial.print(F("Stress mode: start (side="));
      Serial.print((stressSide == 0) ? F("left") : (stressSide == 1) ? F("right")
                                                                     : F("both"));
      Serial.print(F(", delay_us="));
      Serial.print(stressDelayUs);
      Serial.println(F(")"));
      Serial.println(F("Type 'q' to quit, 'd <us>' to change delay, or 'left'/'right'/'both' to switch"));
      continue;
    }
#ifdef UTCMON_SSD1322_PATCH
    if (line.startsWith("ginj"))
    {
      // ginj off
      // ginj a0 [A] [B] [idx] [ns]
      // ginj raw <hex...up to 4> [idx] [ns]
      String rest = line.substring(4);
      rest.trim();
      if (rest == "off")
      {
        u8x8_ssd1322_set_inject(0, 0, nullptr, 0, 0);
        Serial.println(F("Injector OFF"));
        continue;
      }
      // tokenise
      uint8_t bytes[4];
      uint8_t blen = 0;
      uint8_t idx = 12;
      uint16_t ns = 200;
      if (rest.startsWith("randa0"))
      {
        String r = rest.substring(6);
        r.trim();
        if (r.equalsIgnoreCase("off") || r == "0")
        {
          u8x8_ssd1322_inject_rand_a0(0);
          u8x8_ssd1322_inject_randa0_force_b(0, 0x01);
          Serial.println(F("Injector randa0=OFF"));
          continue;
        }
        if (r.length() == 0 || r.equalsIgnoreCase("on") || r == "1")
        {
          u8x8_ssd1322_inject_rand_a0(1);
          u8x8_ssd1322_inject_randa0_force_b(0, 0x01);
          Serial.println(F("Injector randa0=ON (A random, B random)"));
          continue;
        }
        uint8_t Bhex;
        if (!parseHexByte(r, Bhex) || !((Bhex == 0x11) || (Bhex == 0x01)))
        {
          Serial.println(F("ERR: ginj randa0 expects: on|off|11|01"));
          continue;
        }
        u8x8_ssd1322_inject_rand_a0(1);
        u8x8_ssd1322_inject_randa0_force_b(1, Bhex);
        Serial.print(F("Injector randa0=ON (A random, B forced to 0x"));
        printHex2(Bhex);
        Serial.println(F(")"));
        continue;
      }

      if (rest.startsWith("rand"))
      {
        String r = rest.substring(4);
        r.trim();
        if (r.startsWith("idx"))
        {
          r = r.substring(3);
          r.trim();
          if (!r.length())
          {
            Serial.println(F("ERR: ginj rand idx <min> <max>"));
            continue;
          }
          int sp = r.indexOf(' ');
          String smin = (sp < 0) ? r : r.substring(0, sp);
          String smax = (sp < 0) ? String() : r.substring(sp + 1);
          uint32_t vmin = strtoul(smin.c_str(), nullptr, (smin.startsWith("0x") || smin.startsWith("0X")) ? 16 : 10);
          uint32_t vmax = smax.length() ? strtoul(smax.c_str(), nullptr, (smax.startsWith("0x") || smax.startsWith("0X")) ? 16 : 10) : vmin;
          if (vmin > 31)
            vmin = 31;
          if (vmax > 31)
            vmax = 31;
          u8x8_ssd1322_inject_rand_idx((uint8_t)vmin, (uint8_t)vmax);
          Serial.print(F("Injector rand idx in ["));
          Serial.print((unsigned)vmin);
          Serial.print(F(","));
          Serial.print((unsigned)vmax);
          Serial.println(F("]"));
          continue;
        }
        if (r.startsWith("ns"))
        {
          r = r.substring(2);
          r.trim();
          if (!r.length())
          {
            Serial.println(F("ERR: ginj rand ns <min> <max>"));
            continue;
          }
          int sp = r.indexOf(' ');
          String smin = (sp < 0) ? r : r.substring(0, sp);
          String smax = (sp < 0) ? String() : r.substring(sp + 1);
          uint32_t vmin = strtoul(smin.c_str(), nullptr, (smin.startsWith("0x") || smin.startsWith("0X")) ? 16 : 10);
          uint32_t vmax = smax.length() ? strtoul(smax.c_str(), nullptr, (smax.startsWith("0x") || smax.startsWith("0X")) ? 16 : 10) : vmin;
          if (vmin > 100000U)
            vmin = 100000U;
          if (vmax > 100000U)
            vmax = 100000U;
          u8x8_ssd1322_inject_rand_ns((uint16_t)vmin, (uint16_t)vmax);
          Serial.print(F("Injector rand ns in ["));
          Serial.print((unsigned)vmin);
          Serial.print(F(","));
          Serial.print((unsigned)vmax);
          Serial.println(F("]"));
          continue;
        }
        Serial.println(F("ERR: ginj rand idx <min> <max> | ns <min> <max>"));
        continue;
      }
      if (rest.startsWith("a0"))
      {
        bytes[0] = 0xA0;
        blen = 1;
        String r = rest.substring(2);
        r.trim();
        uint8_t vals[4];
        int n = parseHexLine(r, vals, 4);
        if (n >= 1)
        {
          bytes[1] = vals[0];
          blen = 2;
        }
        else
        {
          bytes[1] = 0x06;
          blen = 2;
        } 
        if (n >= 2)
        {
          bytes[2] = vals[1];
          blen = 3;
        }
        else
        {
          bytes[2] = 0x11;
          blen = 3;
        }
        if (n >= 3)
          idx = vals[2];
        if (n >= 4)
          ns = vals[3];
        u8x8_ssd1322_set_inject(1, idx, bytes, blen, ns);
        Serial.print(F("Injector A0 at byte="));
        Serial.print(idx);
        Serial.print(F(" ns="));
        Serial.print(ns);
        Serial.print(F(" A="));
        printHex2(bytes[1]);
        Serial.print(F(" B="));
        printHex2(bytes[2]);
        Serial.println();
        continue;
      }

      if (rest.startsWith("raw"))
      {
        String r = rest.substring(3);
        r.trim();
        uint8_t vals[8];
        int n = parseHexLine(r, vals, 8);
        if (n < 1)
        {
          Serial.println(F("ERR: ginj raw <b0> [b1] [b2] [b3] [idx] [ns]"));
          continue;
        }
        blen = (n > 4) ? 4 : n;
        for (int i = 0; i < blen; i++)
          bytes[i] = vals[i];
        if (n >= 5)
          idx = vals[4];
        if (n >= 6)
          ns = vals[5];
        u8x8_ssd1322_set_inject(1, idx, bytes, blen, ns);
        Serial.print(F("Injector RAW at byte="));
        Serial.print(idx);
        Serial.print(F(" ns="));
        Serial.println(ns);
        continue;
      }
      if (rest.startsWith("prob"))
      {
        // ginj prob <permille 0..1000> [tile|band] [once] [seed]
        String r = rest.substring(4);
        r.trim();
        if (!r.length())
        {
          Serial.println(F("ERR: ginj prob <permille 0..1000> [tile|band] [once] [seed]"));
          continue;
        }
        uint32_t perm = 0;
        uint8_t scope = 0;
        uint8_t once = 0;
        uint32_t seed = 0;
        while (r.length())
        {
          int sp = r.indexOf(' ');
          String tok = (sp < 0) ? r : r.substring(0, sp);
          r = (sp < 0) ? String() : r.substring(sp + 1);
          r.trim();
          String t = tok;
          t.trim();
          if (t.equalsIgnoreCase("tile"))
          {
            scope = 0;
            continue;
          }
          if (t.equalsIgnoreCase("band"))
          {
            scope = 1;
            continue;
          }
          if (t.equalsIgnoreCase("once"))
          {
            once = 1;
            continue;
          }
          char *end = nullptr;
          uint32_t val = strtoul(t.c_str(), &end, (t.startsWith("0x") || t.startsWith("0X")) ? 16 : 10);
          if (perm == 0)
          {
            perm = val;
            if (perm > 1000)
              perm = 1000;
          }
          else
          {
            seed = val;
          }
        }
        u8x8_ssd1322_inject_cfg(1, (uint16_t)perm, scope, once, seed);
        Serial.print(F("Injector PROB set: permille="));
        Serial.print(perm);
        Serial.print(F(" scope="));
        Serial.print(scope ? F("band") : F("tile"));
        Serial.print(F(" once="));
        Serial.print(once);
        if (seed)
        {
          Serial.print(F(" seed=0x"));
          Serial.print(seed, HEX);
        }
        Serial.println();
        continue;
      }

      if (rest == "on")
      {
        u8x8_ssd1322_inject_enable(1);
        Serial.println(F("Injector ENABLED"));
        continue;
      }
      if (rest == "off")
      {
        u8x8_ssd1322_inject_enable(0);
        Serial.println(F("Injector DISABLED"));
        continue;
      }

      Serial.println(F("Usage: ginj off | ginj a0 [A] [B] [idx] [ns] | ginj raw <hex..up to 4> [idx] [ns]"));
      continue;
    }
#endif

    uint8_t bytes[16];
    int nbytes = parseHexLine(line, bytes, 16);
    if (nbytes <= 0)
    {
      Serial.println(F("ERR: expected hex bytes like: A0 FF 11"));
      continue;
    }

    uint8_t cmd = bytes[0];
    int argc = nbytes - 1;

    bool ok = sendCmdWithArgs_Left(cmd, bytes + 1, argc);
    if (ok)
    {
      Serial.print(F("OK: sent "));
      Serial.print(nbytes);
      Serial.print(F(" byte(s):"));
      for (int i = 0; i < nbytes; ++i)
      {
        Serial.print(F(" "));
        printHex2(bytes[i]);
      }
      Serial.println();
    }
    else
    {
      Serial.println(F("ERR: too many args (max 15 here)"));
    }
    delay(1);
  }
}
