#include "DateTime.h"



DateTime::DateTime(String tz_name){
  this->changeTimezone(tz_name);
}
void DateTime::changeTimezone(String tz_name){
  this->tz_name = tz_name;
  this->tz = zoneManager.createForZoneName(tz_name.c_str());
}
int DateTime::getIsoWeekNumber(int year, int month, int day) {
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

DateTimeStruct DateTime::getDateTimeStruct(){
  this->now = time(nullptr);
  this->ldt = LocalDateTime::forUnixSeconds64(this->now);
  this->ze = this->tz.getZonedExtra(this->ldt);
  this->zdt = ZonedDateTime::forUnixSeconds64(this->now, this->tz);

  this->dt.year = this->zdt.year();
  this->dt.month = this->zdt.month();
  this->dt.day = this->zdt.day();
  this->dt.week = this->getIsoWeekNumber(this->zdt.year(), this->zdt.month(), this->zdt.day());
  this->dt.weekday = DateStrings().dayOfWeekLongString(this->zdt.dayOfWeek());
  this->dt.hour = this->zdt.hour();
  this->dt.minute = this->zdt.minute();
  this->dt.second = this->zdt.second();
  this->dt.timezone = this->ze.abbrev();

  this->dt.timestamp = this->now;

  return this->dt;
}