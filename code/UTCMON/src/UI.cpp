#include "UI.h"
#include "common.h"
#include <U8g2lib.h>

UI::UI(DisplayConfig leftConfig, DisplayConfig rightConfig){
  this->leftConfig = leftConfig;
  this->rightConfig = rightConfig;
}
DevicePairInitSuccess UI::init(){
  DevicePairInitSuccess initSuccess;
  this->left = U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, leftConfig.CS, leftConfig.DC, leftConfig.RESET);
  this->left.setBusClock(leftConfig.Frequency);
  initSuccess.left = this->left.begin();
  this->right = U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(U8G2_R0, rightConfig.CS, rightConfig.DC, rightConfig.RESET);
  this->right.setBusClock(rightConfig.Frequency);
  initSuccess.right = this->right.begin();
  return initSuccess;
}
void UI::drawClock(DateTimeStruct dt, int mm_l, int mm_r, int lux_l, int lux_r){
  char buffer[256];

  this->left.clearBuffer();

  // Left screen, bottom line, large: YYYY-MM-DD
  sprintf(buffer, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
  this->left.setFont(u8g2_font_logisoso38_tn);
  this->left.drawStr(0,64,buffer);

  // Left screen, top line, small: ISO week - Weekeday
  sprintf(buffer, "W%02d %.16s", dt.week, dt.weekday);
  this->left.setFont(u8g2_font_logisoso16_tr);
  this->left.drawStr(0,20,buffer);

  // Left screen, top line, small: distance
  if (mm_l<=2500) sprintf(buffer, "% 4d.%d cm", int(mm_l/10), mm_l%10); else sprintf(buffer, " >>>>> cm");
  this->left.setFont(u8g2_font_t0_12_tr);
  this->left.drawStr(140,10,buffer);
  sprintf(buffer, "% 6d lx", lux_l);
  this->left.setFont(u8g2_font_t0_12_tr);
  this->left.drawStr(140,20,buffer);


  if (mm_r<=2500) sprintf(buffer, "% 4d.%d cm", int(mm_r/10), mm_r%10); else sprintf(buffer, " >>>>> cm");
  this->left.setFont(u8g2_font_t0_12_tr);
  this->left.drawStr(200,10,buffer);
  sprintf(buffer, "% 6d lx", lux_r);
  this->left.setFont(u8g2_font_t0_12_tr);
  this->left.drawStr(200,20,buffer);

  this->left.sendBuffer();

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

  this->right.sendBuffer();
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
  this->left.sendBuffer();

  this->right.clearBuffer();
  this->right.drawBox(0,0,256,64);
  this->right.setDrawColor(0);
  this->right.setFont(u8g2_font_logisoso32_tr);
  this->right.drawStr(0,32,"UTCMON Right");
  this->right.setFont(u8g2_font_profont17_mr);
  this->right.drawStr(0,60,"Initializing................................");
  this->right.setDrawColor(1);
  this->right.sendBuffer();
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
  this->left.sendBuffer();
}
void UI::drawInitScreenNetPhase1(String ssid){
  // TODO: support retry/wait count display
  this->right.clearBuffer();
  this->right.drawStr(0,20,"SSID: ");
  this->right.drawStr(48,20,ssid.c_str());
  this->right.drawStr(0,40,"Wi-Fi connecting...");
  this->right.sendBuffer();
}
void UI::drawInitScreenNetPhase2(){
  this->right.drawStr(0,40,"Wi-Fi connected       ");
  this->right.drawStr(0,60,"NTP initializing...");
  this->right.sendBuffer();
}
void UI::drawInitScreenNetPhase3(){
  this->right.drawStr(0,60,"NTP initiliazing...");
  this->right.sendBuffer();
}
void UI::drawInitScreenNetPhase4(){
  this->right.drawStr(0,60,"NTP synced              ");
  this->right.sendBuffer();
}

void UI::setContrast(int contrast){
  this->left.setContrast(contrast);
  this->right.setContrast(contrast);
}