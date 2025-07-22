#pragma once
#include <Arduino.h>
#include <time.h>
#include <WiFiUdp.h>

struct NtpDiagnostics {
  time_t lastNtpSync = 0;
  time_t lastDriftCheck = 0;
  uint64_t lastDriftMs = 0;
  //sntp_sync_status_t syncStatus = SNTP_SYNC_STATUS_RESET;
};
extern NtpDiagnostics ntpDiagnostics; // TODO: should that be global or in some class?
extern WiFiUDP udp;
extern "C" {
  #include "esp_sntp.h"
}

// helpers for ESP-IDF managed time synchronization
void timeSyncNotificationCB(struct timeval *tv);
void configTimeExtended(long gmtOffset_sec, int daylightOffset_sec, const char* ntpServer);

// manual NTP diagnostics (ESP-IDF does not provide drift info so we need to calculate it ourselves)
void sendNtpPpacket(const char* address);
time_t getNtpTime();

// diagnostics
void checkNTP();
int minutesSinceLastNtpSync();
int secondsSinceLastNtpCheck();
