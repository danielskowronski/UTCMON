#pragma once

#include <Arduino.h>
#include <AceTime.h>
#include <time.h>

using namespace ace_time;
static ExtendedZoneProcessorCache<3> zoneProcessorCache;
static ExtendedZoneManager zoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    zoneProcessorCache);

struct DateTimeStruct {
  int year;
  int month;
  int day;
  int week;
  String weekday;
  int hour;
  int minute;
  int second;
  String timezone;
  uint64_t timestamp;
};

class DateTime {
public:
  DateTime(String tz="UTC");
  void changeTimezone(String tz_name);
  DateTimeStruct getDateTimeStruct();
private:
  int getIsoWeekNumber(int year, int month, int day);
  DateTimeStruct dt;
  String tz_name;
  ace_time::TimeZone tz;
  time_t now;
  ace_time::LocalDateTime ldt;
  ace_time::ZonedExtra ze;
  ace_time::ZonedDateTime zdt;
};