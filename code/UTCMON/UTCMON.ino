#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_l(U8G2_R0, /* cs=*/ 5, /* dc=*/ 0, /* reset=*/ 4);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_r(U8G2_R0, /* cs=*/ 17, /* dc=*/ 2, /* reset=*/ 16);

void drawClock(int year, int month, int day, int week, char* weekday, int hour, int minute, int second, char* timezone){
  char buffer[256];

  u8g2_l.clearBuffer();

  // Left screen, bottom line, large: YYYY-MM-DD
  sprintf(buffer, "%04d-%02d-%02d", year%10000, month%100, day%100);
  u8g2_l.setFont(u8g2_font_logisoso38_tn);
  u8g2_l.drawStr(0,64,buffer);

  // Left screen, top line, small: ISO week - Weekeday
  sprintf(buffer, "W%02D %.16s", week%100, weekday);
  u8g2_l.setFont(u8g2_font_logisoso16_tr);
  u8g2_l.drawStr(0,20,buffer);

  u8g2_l.sendBuffer();
  
  u8g2_r.clearBuffer();

  // Right screen, bottom line, large: HH:MM
  sprintf(buffer, "%02d:%02d", hour%100, minute%100);
  u8g2_r.setFont(u8g2_font_logisoso62_tn);
  u8g2_r.drawStr(0,64,buffer);

  // Right screen, bottom line continued, medium: :SS
  sprintf(buffer, ":%02d", second%100);
  u8g2_r.setFont(u8g2_font_logisoso46_tn);
  u8g2_r.drawStr(5*36,64,buffer);

  // Right screen, top line, above :SS, small: timezone
  sprintf(buffer, "%.4s", timezone);
  u8g2_r.setFont(u8g2_font_logisoso16_tr);
  u8g2_r.drawStr(5*36+30+5*(strlen(timezone)%2),16,buffer);

  u8g2_r.sendBuffer();
}

void setup() {
  Serial.begin(9600);
  SPI.setFrequency(15000000);
  u8g2_l.begin();
  u8g2_r.begin();
}

void loop() {
  drawClock(2025,3,20,12,"Wednesday", 21,37,12, "UTC");
  sleep(2);
  drawClock(2025,12,1,1,"Friday", 11,11,11, "CEST");
  sleep(2);
}
