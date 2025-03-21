#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>
#include "Secrets.h"

using namespace ace_time;
static const int CACHE_SIZE = 3;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_l(U8G2_R0, /* cs=*/ 5, /* dc=*/ 0, /* reset=*/ 4);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_r(U8G2_R0, /* cs=*/ 17, /* dc=*/ 2, /* reset=*/ 16);

int computeIsoWeekNumber(int year, int month, int day) {
  // Build a tm structure for the given date at noon to avoid DST edge issues.
  struct tm tmDate = {0};
  tmDate.tm_year = year - 1900;
  tmDate.tm_mon  = month - 1;
  tmDate.tm_mday = day;
  tmDate.tm_hour = 12; 
  mktime(&tmDate);
  
  // Determine ISO weekday (Monday=1, Tuesday=2, …, Sunday=7)
  int isoWeekday = (tmDate.tm_wday == 0) ? 7 : tmDate.tm_wday;
  
  // Shift the date to the Thursday of the current week.
  time_t t = mktime(&tmDate);
  time_t t_thursday = t + (4 - isoWeekday) * 86400; // 86400 seconds per day
  struct tm tmThursday;
  localtime_r(&t_thursday, &tmThursday);
  int isoYear = tmThursday.tm_year + 1900;
  
  // Compute January 4th of the ISO year.
  struct tm tmJan4 = {0};
  tmJan4.tm_year = isoYear - 1900;
  tmJan4.tm_mon  = 0; // January
  tmJan4.tm_mday = 4;
  tmJan4.tm_hour = 12;
  mktime(&tmJan4);
  int jan4IsoWeekday = (tmJan4.tm_wday == 0) ? 7 : tmJan4.tm_wday;
  
  // Find the Monday of the week containing January 4 (the start of week 1).
  time_t t_monday = mktime(&tmJan4) - (jan4IsoWeekday - 1) * 86400;
  
  // Compute the week number as the number of whole weeks between t_monday and t_thursday.
  int weekNumber = 1 + ((int)(t_thursday - t_monday) / 86400) / 7;
  return weekNumber;
}


void drawClock(int year, int month, int day, int week, const char* weekday, int hour, int minute, int second, const char* timezone){
  char buffer[256];

  u8g2_l.clearBuffer();

  // Left screen, bottom line, large: YYYY-MM-DD
  sprintf(buffer, "%04d-%02d-%02d", year%10000, month%100, day%100);
  u8g2_l.setFont(u8g2_font_logisoso38_tn);
  u8g2_l.drawStr(0,64,buffer);

  // Left screen, top line, small: ISO week - Weekeday
  sprintf(buffer, "W%02d %.16s", week, weekday);
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
  u8g2_r.drawStr(5*36+26+5*(strlen(timezone)%2),16,buffer);

  u8g2_r.sendBuffer();
}


void setup() {
  Serial.begin(9600);
  SPI.setFrequency(15000000);
  u8g2_l.begin();
  u8g2_r.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting ...");
    delay(500);
    }
  Serial.println("Connected");  
  configTime(0, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for time sync...");
    delay(1000);
  }
}

String TZ="Europe/Warsaw"; //to be controlled later
void loop() {
  time_t now = time(nullptr);
  LocalDateTime ldt = LocalDateTime::forUnixSeconds64(now);
  
  TimeZone tz = manager.createForZoneName(TZ.c_str());
  ZonedExtra ze = tz.getZonedExtra(ldt);
  ZonedDateTime zdt = ZonedDateTime::forUnixSeconds64(now, tz);

  int isoWeekNumber = computeIsoWeekNumber(zdt.year(),  zdt.month(), zdt.day());
  drawClock(zdt.year(), zdt.month(), zdt.day(), isoWeekNumber, DateStrings().dayOfWeekLongString(zdt.dayOfWeek()), zdt.hour(), zdt.minute(), zdt.second(), ze.abbrev());

  sleep(0.5);
}
