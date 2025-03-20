#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_l(U8G2_R0, /* cs=*/ 5, /* dc=*/ 0, /* reset=*/ 4);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_r(U8G2_R0, /* cs=*/ 17, /* dc=*/ 2, /* reset=*/ 16);


void setup() {
  Serial.begin(9600);
  u8g2_l.begin();
  u8g2_r.begin();
}

int i=0;
void loop() {
  Serial.println(i);

  u8g2_l.clearBuffer();
  u8g2_l.setFont(u8g2_font_spleen32x64_me);
  u8g2_l.drawStr(i,40,"left");
  u8g2_l.sendBuffer();

  u8g2_r.clearBuffer();
  u8g2_r.setFont(u8g2_font_spleen32x64_me);
  u8g2_r.drawStr(i,40,"right");
  u8g2_r.sendBuffer();

  i++;
  if (i>=256) i=0;

  sleep(0.1);
}
