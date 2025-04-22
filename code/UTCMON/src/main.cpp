#define VERSION "v0.2.1"
#ifndef BUILD_DATE
  #define BUILD_DATE "YYYY-MM-DD"
#endif
#define VER_INFO VERSION " " BUILD_DATE

#include <Arduino.h>

#include "hw_config.h"

#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <AceTime.h>
#include <time.h>

#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>


using namespace ace_time;
static const int CACHE_SIZE = 3;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

TwoWire I2C_l = TwoWire(LeftBus::I2C::Bus);
TwoWire I2C_r = TwoWire(RightBus::I2C::Bus);

Adafruit_VL53L0X lox_l = Adafruit_VL53L0X();
Adafruit_VL53L0X lox_r = Adafruit_VL53L0X();

Adafruit_TSL2561_Unified tsl_l = Adafruit_TSL2561_Unified(LeftBus::LightSensor::I2CAddress);
Adafruit_TSL2561_Unified tsl_r = Adafruit_TSL2561_Unified(RightBus::LightSensor::I2CAddress);

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_l(U8G2_R0, LeftBus::Display::CS,  LeftBus::Display::DC,  LeftBus::Display::RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2_r(U8G2_R0, RightBus::Display::CS, RightBus::Display::DC, RightBus::Display::RESET);


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


void drawClock(int year, int month, int day, int week, const char* weekday, int hour, int minute, int second, const char* timezone, int mm_l, int mm_r, int lux_l, int lux_r){
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

  // Left screen, top line, small: distance
  if (mm_l<=2500) sprintf(buffer, "% 4d.%d cm", int(mm_l/10), mm_l%10); else sprintf(buffer, " >>>>> cm");
  u8g2_l.setFont(u8g2_font_t0_12_tr);
  u8g2_l.drawStr(140,10,buffer);
  sprintf(buffer, "% 6d lx", lux_l);
  u8g2_l.setFont(u8g2_font_t0_12_tr);
  u8g2_l.drawStr(140,20,buffer);


  if (mm_r<=2500) sprintf(buffer, "% 4d.%d cm", int(mm_r/10), mm_r%10); else sprintf(buffer, " >>>>> cm");
  u8g2_l.setFont(u8g2_font_t0_12_tr);
  u8g2_l.drawStr(200,10,buffer);
  sprintf(buffer, "% 6d lx", lux_r);
  u8g2_l.setFont(u8g2_font_t0_12_tr);
  u8g2_l.drawStr(200,20,buffer);

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
  Serial.begin(CommonBus::Serial::Baudrate);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  SPI.setFrequency(CommonBus::SPI::Frequency);
  u8g2_l.setBusClock(LeftBus::Display::Frequency);
  u8g2_r.setBusClock(RightBus::Display::Frequency);
  u8g2_l.begin();
  u8g2_r.begin();

  u8g2_l.clearBuffer();
  u8g2_l.drawBox(0,0,256,64);
  u8g2_l.setDrawColor(0);
  u8g2_l.setFont(u8g2_font_logisoso32_tr);
  u8g2_l.drawStr(0,32,"UTCMON Left");
  u8g2_l.setFont(u8g2_font_profont17_mr);
  u8g2_l.drawStr(0,60,VER_INFO);
  u8g2_l.setDrawColor(1);
  u8g2_l.sendBuffer();
  

  u8g2_r.clearBuffer();
  u8g2_r.drawBox(0,0,256,64);
  u8g2_r.setDrawColor(0);
  u8g2_r.setFont(u8g2_font_logisoso32_tr);
  u8g2_r.drawStr(0,32,"UTCMON Right");
  u8g2_r.setFont(u8g2_font_profont17_mr);
  u8g2_r.drawStr(0,60,"Initializing................................");
  u8g2_r.setDrawColor(1);
  u8g2_r.sendBuffer();

  Wire.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL);
  Wire1.begin(RightBus::I2C::SDA, RightBus::I2C::SCL);
  I2C_l.begin(LeftBus::I2C::SDA, LeftBus::I2C::SCL, LeftBus::I2C::Frequency);
  I2C_r.begin(RightBus::I2C::SDA, RightBus::I2C::SCL, RightBus::I2C::Frequency); 

  delay(250);
  bool lox_ok_l = lox_l.begin(LeftBus::DistanceSensor::I2CAddress, LeftBus::DistanceSensor::DebugEnabled, &Wire);
  Serial.printf("lox_l.begin() %1d\n", lox_ok_l);
  if (lox_ok_l) lox_l.configSensor(LeftBus::DistanceSensor::Mode);
  delay(250);
  bool lox_ok_r = lox_r.begin(RightBus::DistanceSensor::I2CAddress, RightBus::DistanceSensor::DebugEnabled, &Wire1);
  Serial.printf("lox_r.begin() %1d\n", lox_ok_r);
  if (lox_ok_r) lox_r.configSensor(RightBus::DistanceSensor::Mode);
  delay(250);

  bool tsl_ok_l = tsl_l.begin(&Wire);
  Serial.printf("tsl_l.begin() %1d\n", tsl_ok_l);
  delay(250);
  bool tsl_ok_r = tsl_r.begin(&Wire1);
  Serial.printf("tsl_r.begin() %1d\n", tsl_ok_r);
  delay(250);

  char buffer[256];
  u8g2_l.clearBuffer();
  u8g2_l.drawStr(0,20,"UTCMON ver: ");u8g2_l.drawStr(100,20,VER_INFO);
  u8g2_l.drawStr(0,40,"Dist. sensors: ");
  if (lox_ok_l) u8g2_l.drawStr(130,40,"_OK_"); else u8g2_l.drawStr(130,40,"FAIL"); 
  if (lox_ok_r) u8g2_l.drawStr(180,40,"_OK_"); else u8g2_l.drawStr(180,40,"FAIL"); 
  u8g2_l.drawStr(0,60,"Light sensors: ");
  if (tsl_ok_l) u8g2_l.drawStr(130,60,"_OK_"); else u8g2_l.drawStr(130,60,"FAIL"); 
  if (tsl_ok_r) u8g2_l.drawStr(180,60,"_OK_"); else u8g2_l.drawStr(180,60,"FAIL"); 
  u8g2_l.sendBuffer();


  u8g2_r.clearBuffer();
  u8g2_r.setFontMode(0);
  u8g2_r.setDrawColor(1);
  u8g2_r.drawStr(0,20,"SSID: ");
  u8g2_r.drawStr(48,20,WIFI_SSID);
  u8g2_r.sendBuffer();


  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting ...");
    u8g2_r.drawStr(0,40,"Wi-Fi connecting...");
    u8g2_r.sendBuffer();
    delay(500);
    }
  


  u8g2_r.drawStr(0,40,"Wi-Fi connected       ");
  u8g2_r.sendBuffer();
  Serial.println("Connected");
  configTime(0, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {

    u8g2_r.drawStr(0,60,"NTP initiliazing...");
    u8g2_r.sendBuffer();
    Serial.println("Waiting for time sync...");
    delay(1000);
  }

    u8g2_r.drawStr(0,60,"NTP synced              ");
    u8g2_r.sendBuffer();
    delay(2000);
}

String TZ="Europe/Warsaw"; //to be controlled later


VL53L0X_RangingMeasurementData_t measure_l;
VL53L0X_RangingMeasurementData_t measure_r;
sensors_event_t event_l;
sensors_event_t event_r;
int avg_lux;
int contrast=255;

void loop() {
  time_t now = time(nullptr);
  LocalDateTime ldt = LocalDateTime::forUnixSeconds64(now);

  TimeZone tz = manager.createForZoneName(TZ.c_str());
  ZonedExtra ze = tz.getZonedExtra(ldt);
  ZonedDateTime zdt = ZonedDateTime::forUnixSeconds64(now, tz);

  int isoWeekNumber = computeIsoWeekNumber(zdt.year(),  zdt.month(), zdt.day());

  lox_l.rangingTest(&measure_l, false);
  lox_r.rangingTest(&measure_r, false);

  tsl_l.getEvent(&event_l);
  tsl_r.getEvent(&event_r);
  avg_lux = int((int(event_l.light)+int(event_r.light))/2);
  if (avg_lux>512) contrast=255; else contrast=int(avg_lux/2);
  Serial.printf("lux: L=%05d R=%05d AVG=%05d => contrast=%03d\n", int(event_l.light), int(event_r.light), avg_lux, contrast);
  u8g2_l.setContrast(contrast);
  u8g2_r.setContrast(contrast);


  drawClock(zdt.year(), zdt.month(), zdt.day(), isoWeekNumber, DateStrings().dayOfWeekLongString(zdt.dayOfWeek()), zdt.hour(), zdt.minute(), zdt.second(), ze.abbrev(), int(measure_l.RangeMilliMeter), int(measure_r.RangeMilliMeter), int(event_l.light), int(event_r.light));

  delay(250); 
}
