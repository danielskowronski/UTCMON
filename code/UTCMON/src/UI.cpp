#include "UI.h"
#include "common.h"
#include <U8g2lib.h>
#include "TimeSync.h"
#include "Logging.h"

UI::UI(DisplayConfig leftConfig, DisplayConfig rightConfig){
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
void UI::sendBuffer(U8G2 &screen) {
  noInterrupts();
  screen.sendBuffer();
  interrupts();
}
DevicePairInitSuccess UI::init(){
  DevicePairInitSuccess initSuccess;
  this->left = U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, leftConfig.CS, leftConfig.DC, leftConfig.RESET);
  this->left.setBusClock(leftConfig.Frequency);
  initSuccess.left = this->left.begin();
  
  this->right = U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, rightConfig.CS, rightConfig.DC, rightConfig.RESET);
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
void UI::drawClock(DateTimeStruct dt, DistanceStatusPair dsp, int lux_l, int lux_r){
  char buffer[256];

  this->left.clearBuffer();

  switch (this->dateDisplayMode){
    case FullOnlyDate:
    case FullAndNTP:
    case FullAndSensors:
      // Left screen, bottom line, large: YYYY-MM-DD
      sprintf(buffer, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
      this->left.setFont(u8g2_font_logisoso38_tn);
      this->left.drawStr(0,64,buffer);

      // Left screen, top line, small: ISO week - Weekeday
      sprintf(buffer, "W%02d %.16s", dt.week, dt.weekday);
      this->left.setFont(u8g2_font_logisoso16_tr);
      this->left.drawStr(0,20,buffer);
      break;
  }

  switch (this->dateDisplayMode){
    case FullAndSensors:
        // Left screen, top line, small: distance
        this->left.setFont(u8g2_font_t0_12_tr);
        this->left.drawStr(140,10,DistanceSensors::fmtDist(dsp.left, true, true).c_str());
        sprintf(buffer, "% 5dlux", lux_l);
        this->left.setFont(u8g2_font_t0_12_tr);
        this->left.drawStr(140,20,buffer);

        this->left.setFont(u8g2_font_t0_12_tr);
        this->left.drawStr(200,10,DistanceSensors::fmtDist(dsp.right, true, true).c_str());
        sprintf(buffer, "% 5dlux", lux_r);
        this->left.setFont(u8g2_font_t0_12_tr);
        this->left.drawStr(200,20,buffer);
      break;
    case FullAndNTP:
      this->left.setFont(u8g2_font_t0_12_tr);
      sprintf(buffer, "drift:     %4dms", ntpDiagnostics.lastDriftMs);
      this->left.drawStr(150,10,buffer);
      sprintf(buffer, "sync: %3d min ago", minutesSinceLastNtpSync());
      this->left.drawStr(150,20,buffer);
      break;
    default:
      // No additional info
      break;
  }

  this->sendBuffer(this->left);

  //while (digitalRead(this->rightConfig.CS) == LOW) {
  //  logger.warn(TAG_DISPLAYS, "drawClock: waiting for Right CS to be released");
  //  delayMicroseconds(100);
  //}

  this->right.clearBuffer();

  // Right screen, bottom line, large: HH:MM
  sprintf(buffer, "%02d:%02d", dt.hour, dt.minute);
  this->right.setFont(u8g2_font_logisoso62_tn);
  this->right.drawStr(0,64,buffer);

  // Right screen, bottom line continued, medium: :SS
  sprintf(buffer, ":%02d", dt.second);
  this->right.setFont(u8g2_font_logisoso46_tn);
  this->right.drawStr(5*36,64,buffer);

  // Right screen, top line, above :SS, small: timezone
  sprintf(buffer, "%.4s", dt.timezone);
  this->right.setFont(u8g2_font_logisoso16_tr);
  this->right.drawStr(5*36+26+5*(strlen(dt.timezone.c_str())%2),16,buffer);

  this->sendBuffer(this->right);
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
  this->left.drawStr(0,20,"UTCMON ver: ");
  this->left.drawStr(100,20,version.c_str());
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