#include "UI.h"
#include "common.h"
#include "TimeSync.h"
#include "Logging.h"
#include "WiFi.h"
#include <U8g2lib.h> 
#include <new>

#include "Fonts.h"


void UI::setFont(U8G2 &screen, u8_t size){
  // FIXME: ensure all setFonts are set in the same way, also support different font variants (text, graphic, different charset) + all those OIL_FF
  this->setFont(screen, size, this->commonDisplayMode==Hollow);
}
void UI::setFont(U8G2 &screen, u8_t size, bool outline) {
  if (outline) {
    switch (size) {
      case 62: screen.setFont(u8g2_font_logisoso62_outline_tn); break; // u8g2_font_spleen32x64_mn 
      //case 46: screen.setFont(u8g2_font_logisoso46_outline_tn); break;
      case 38: screen.setFont(u8g2_font_logisoso38_outline_tn); break;
      case 16: screen.setFont(u8g2_font_calibration_gothic_nbp_tr); break;
      default: screen.setFont(u8g2_font_logisoso16_tr); break; // fallback
    }
  } else {
    switch (size) {
      case 62: screen.setFont(u8g2_font_logisoso62_orig_tn); break;
      case 46: screen.setFont(u8g2_font_logisoso46_tn); break;
      case 38: screen.setFont(u8g2_font_logisoso38_orig_tn); break;
      case 16: screen.setFont(u8g2_font_logisoso16_tr); break;
      default: screen.setFont(u8g2_font_logisoso16_tr); break; // fallback
    }
  }
}
UI::UI(DisplayConfig leftConfig, DisplayConfig rightConfig){
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
void UI::sendBuffer(U8G2 &screen) {
  //noInterrupts();
  // u8x8_d_ssd1322_common
  if (this->commonDisplayMode == Hollow) {
    
  }
  screen.sendBuffer();
  //interrupts();
}
DevicePairInitSuccess UI::init(){
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
void UI::resetOneScreen(U8G2 &screen){
  screen.clearBuffer();
  this->sendBuffer(screen);

  delayMicroseconds(1*1000);

  screen.setDrawColor(1);
  screen.drawBox(0, 0, 256, 64);
  this->sendBuffer(screen);

  delayMicroseconds(1*1000);

  screen.clearBuffer();
  this->sendBuffer(screen);
}
void UI::resetScreens(){
  this->resetOneScreen(this->left);
  this->resetOneScreen(this->right);
}
void UI::setDateDisplayMode(DateDisplayMode mode) {
  this->dateDisplayMode = mode;
}
void UI::setCommonDisplayMode(CommonDisplayMode mode) {
  this->commonDisplayMode = mode;
}
CommonDisplayMode UI::getCommonDisplayMode(){
  return this->commonDisplayMode;
}
bool UI::needToRedrawDate(DisplayContentsDate newDisplayContentsDate) {
  #ifdef STRESS_TEST_DRAW
  return true;
  #endif
  bool need=false;
  if (this->displayContentsTime.commonDisplayMode==Blank && newDisplayContentsDate.commonDisplayMode==Blank) {
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
      this->displayContentsDate.commonDisplayMode != newDisplayContentsDate.commonDisplayMode
  ) {
    need = true;
  }
  return need;
}
void UI::setInvert(U8G2 &screen, bool enable) {
  if (enable) {
    screen.setDrawColor(1);
    screen.drawBox(0, 0, DISP_W, DISP_H);
    screen.setDrawColor(0);
  } else {
    screen.setDrawColor(1);
  }
}
uint32_t UI::drawClockDate(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r, uint16_t netIcon){
  // FIXME: check if dt has valid date, if not - do not draw anything or draw "no date" message (issue happens on first draw after init)
  char buffer[256];
  uint32_t beforeFirst, afterFirst, deltaFirst;

  this->left.clearBuffer();
  this->setInvert(this->left, dsp.left.triggering);

  switch (this->dateDisplayMode){
    case FullOnlyDate:
    case FullAndNTP:
    case FullAndSensors:
      // Left screen, bottom line, large: YYYY-MM-DD
      sprintf(buffer, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
      this->setFont(this->left, 38); //this->left.setFont(UIL_DAT_FF);
      this->left.drawStr(UIL_DAT_X,UIL_DAT_Y,buffer);
      // Left screen, top line, small: status icon ()
      this->left.setFont(UIL_ICN_FF);
      this->left.drawGlyph(UIL_ICN_X,UIL_ICN_Y,netIcon);

      // Left screen, top line, small: ISO week - Weekeday
      sprintf(buffer, "W%02d %.16s", dt.week, dt.weekday); // "Wednesday"
      this->setFont(this->left, 16); //this->left.setFont(UIL_TOP_FF);
      this->left.drawStr(UIL_TOP_X,UIL_TOP_Y,buffer);

      break;
  }

  if (this->commonDisplayMode!=Hollow){
    switch (this->dateDisplayMode){
      case FullAndSensors:
          // Left screen, top line, small: distance
          this->left.setFont(UIL_INF_FF);
          this->left.drawStr(UIL_SEN_X1,UIL_INF_FH,DistanceSensor::fmtDist(dsp.left, true, true, true).c_str());
          sprintf(buffer, "%05dlx", lux_l);
          this->left.setFont(UIL_INF_FF);
          this->left.drawStr(UIL_SEN_X1,UIL_INF_FH*2,buffer);

          this->left.drawBox(UIL_SEN_X2-UIL_SEN_COL_PADDING, UIL_SEN_Y0, UIL_SEN_COL_MARGIN, UIL_SEN_H);

          this->left.setFont(UIL_INF_FF);
          this->left.drawStr(UIL_SEN_X2,UIL_INF_FH,DistanceSensor::fmtDist(dsp.right, true, true, true).c_str());
          sprintf(buffer, "%05dlx", lux_r);
          this->left.setFont(UIL_INF_FF);
          this->left.drawStr(UIL_SEN_X2,UIL_INF_FH*2,buffer);
        break;
      case FullAndNTP:
        this->left.setFont(UIL_INF_FF);
        sprintf(buffer, "drift % 5dms", ntpDiagnostics.lastDriftMs);
        this->left.drawStr(UIL_NTP_X,UIL_INF_FH,buffer);
        sprintf(buffer, "sync % 3dm ago", minutesSinceLastNtpSync());
        this->left.drawStr(UIL_NTP_X,UIL_INF_FH*2,buffer);
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
uint32_t UI::drawClockTime(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r){
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
  this->right.drawStr(0,64-1,buffer);
  sprintf(buffer, ":");
  this->setFont(this->right, 62);
  this->right.drawStr(2*36+12,64-1,buffer);
  sprintf(buffer, "%02d", dt.minute);
  this->setFont(this->right, 62);
  this->right.drawStr(3*36,64-1,buffer);

  // Right screen, bottom line continued, medium: :SS
  sprintf(buffer, ":%02d", dt.second);
  this->setFont(this->right, 38); // 46
  this->right.drawStr(5*36+(8),64-1,buffer);

  // Right screen, top line, above :SS, small: timezone
  sprintf(buffer, "%.4s", dt.timezone);
  this->setFont(this->right, 16);
  this->right.drawStr(5*36+26+5*(strlen(dt.timezone.c_str())%2),16+4,buffer);

  beforeSecond = micros();
  this->sendBuffer(this->right);
  afterSecond = micros();

  deltaSecond = afterSecond - beforeSecond;
  return deltaSecond;
}
bool UI::needToRedrawTime(DisplayContentsTime newDisplayContentsTime) {
  #ifdef STRESS_TEST_DRAW
  return true;
  #endif
  bool need=false;
  if (this->displayContentsTime.commonDisplayMode==Blank && newDisplayContentsTime.commonDisplayMode==Blank) {
    need = false;
  }
  else if (this->displayContentsTime.hour != newDisplayContentsTime.hour ||
      this->displayContentsTime.minute != newDisplayContentsTime.minute ||
      this->displayContentsTime.second != newDisplayContentsTime.second ||
      this->displayContentsTime.timezone != newDisplayContentsTime.timezone ||
      this->displayContentsTime.colorsInverted != newDisplayContentsTime.colorsInverted || 
      this->displayContentsTime.commonDisplayMode != newDisplayContentsTime.commonDisplayMode
  ) {
    need = true;
  }
  return need;
}
uint32_t UI::drawBlank(U8G2 &screen) {
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
void UI::drawClock(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r){
  char buffer[256];
  uint32_t beforeFirst, afterFirst, beforeSecond, afterSecond, deltaFirst=0, deltaSecond=0, jitter;

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
    .colorsInverted = dsp.left.triggering
  };
  DisplayContentsTime newDisplayContentsTime = {
    .commonDisplayMode = this->commonDisplayMode,
    .hour = dt.hour,
    .minute = dt.minute,
    .second = dt.second,
    .timezone = dt.timezone,
    .colorsInverted = dsp.right.triggering
  };

  switch (this->dateDisplayMode){
    case FullOnlyDate:
    case FullAndNTP:
      needToRedrawDate = this->needToRedrawDate(newDisplayContentsDate);
      break;
    case FullAndSensors:
      needToRedrawDate = true; // sensors always change
      break;
  }
  this->displayContentsDate = newDisplayContentsDate;
  if (needToRedrawDate) {
    if (this->commonDisplayMode==Blank) deltaFirst = this->drawBlank(this->left);
    else deltaFirst = this->drawClockDate(dt, dsp, lux_l, lux_r, netIcon);
  }

  needToRedrawTime = this->needToRedrawTime(newDisplayContentsTime);
  this->displayContentsTime = newDisplayContentsTime;
  if (needToRedrawTime) {
    if (this->commonDisplayMode==Blank) deltaSecond = this->drawBlank(this->right);
    else deltaSecond = this->drawClockTime(dt, dsp, lux_l, lux_r);
  }

  jitter = abs((int)deltaFirst-(int)deltaSecond);
  bool abnoramJitter = (jitter > DISPLAY_SENDBUFFER_JITTER_WARN_US);
  bool unexpectedTimeFirst = (deltaFirst > DISPLAY_SENDBUFFER_DURATION_WARN_US);
  bool unexpectedTimeSecond = (deltaSecond > DISPLAY_SENDBUFFER_DURATION_WARN_US);
  if (deltaFirst == 0 || deltaSecond == 0) {
    abnoramJitter = false; // if skipping one of the displays, then jitter cannot be calculated
  } 

  // if any of displays take more than DISPLAY_SENDBUFFER_DURATION_WARN_US to update or if the jitter is more than DISPLAY_SENDBUFFER_JITTER_WARN_US then log a warning, else log as verbose-debug
  int logLevel = (abnoramJitter||unexpectedTimeFirst||unexpectedTimeSecond) ? ARDUHAL_LOG_LEVEL_WARN : ARDUHAL_LOG_LEVEL_VERBOSE;
  #ifndef STRESS_TEST_DRAW // with debug, actual display time will always be high
  logger.log(logLevel, TAG_DISPLAYS, "drawClock: left sendBuffer took %8ld us, right sendBuffer took %8ld us, jitter was %8ld us", deltaFirst, deltaSecond, jitter);
  #endif
}
void UI::drawSplashScreen(String version){
  this->left.clearBuffer();
  this->left.drawBox(0,0,256,64);
  this->left.setDrawColor(0);
  this->left.setFont(u8g2_font_logisoso32_tr);
  this->left.drawStr(0,32,"UTCMON Left");
  this->left.setFont(u8g2_font_profont17_mr);
  this->left.drawStr(0,60,version.c_str());
  this->left.setDrawColor(1);
  this->sendBuffer(this->left);

  this->right.clearBuffer();
  this->right.drawBox(0,0,256,64);
  this->right.setDrawColor(0);
  this->right.setFont(u8g2_font_logisoso32_tr);
  this->right.drawStr(0,32,"UTCMON Right");
  this->right.setFont(u8g2_font_profont17_mr);
  this->right.drawStr(0,60,"Initializing................................");
  this->right.setDrawColor(1);
  this->sendBuffer(this->right);
}
void UI::drawInitScreenSensor(String version, bool leftDistanceSensor, bool leftLightSensor, bool rightDistanceSensor, bool rightLightSensor){
  // TODO: fix this monstrosity (ported from proof of concept)
  char buffer[256];

  this->left.clearBuffer();
  this->left.drawStr(0,20,"UTCMON ");
  this->left.drawStr(70,20,version.c_str());
  this->left.drawStr(0,40,"Dist. sensors: ");
  if (leftDistanceSensor) this->left.drawStr(130,40,"_OK_"); else this->left.drawStr(130,40,"FAIL"); 
  if (rightDistanceSensor) this->left.drawStr(180,40,"_OK_"); else this->left.drawStr(180,40,"FAIL"); 
  this->left.drawStr(0,60,"Light sensors: ");
  if (leftLightSensor) this->left.drawStr(130,60,"_OK_"); else this->left.drawStr(130,60,"FAIL"); 
  if (rightDistanceSensor) this->left.drawStr(180,60,"_OK_"); else this->left.drawStr(180,60,"FAIL"); 
  this->sendBuffer(this->left);
}
void UI::drawInitScreenNetPhase1(String ssid){
  // TODO: support retry/wait count display
  char buffer[64];
  sprintf(buffer, "SSID: %s", ssid.c_str());
  this->right.setFont(u8g2_font_profont17_mr);
  this->right.clearBuffer();
  this->right.drawStr(0,20,buffer);
  this->right.drawStr(0,40,"WiFi: connecting...");
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase2(String ipAddress){
  char buffer[64];
  sprintf(buffer, "WiFi: %s", ipAddress.c_str());
  this->right.drawStr(0,40,buffer);
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase3(){
  this->right.drawStr(0,60," NTP: initiliazing...");
  this->sendBuffer(this->right);
}
void UI::drawInitScreenNetPhase4(int driftMs){
  char buffer[64];
  sprintf(buffer, " NTP: synced, drift %d ms", driftMs);
  this->right.drawStr(0,60,buffer);
  this->sendBuffer(this->right);
}

void UI::setContrast(int contrast){
  this->left.setContrast(contrast);
  this->right.setContrast(contrast);
}