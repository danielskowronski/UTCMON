#include "TimeSync.h"
#include "hw_config.h"
#include "Logging.h"
NtpDiagnostics ntpDiagnostics;
WiFiUDP udp;

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

void timeSyncNotificationCB(struct timeval *tv) {
  time(&(ntpDiagnostics.lastNtpSync));
  logger.info(TAG_NTP_SYNC, "NTP sync notification received");
}
void configTimeExtended(long gmtOffset_sec, int daylightOffset_sec, const char* ntpServer) {
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char*)ntpServer); 
  sntp_set_time_sync_notification_cb(timeSyncNotificationCB);
  sntp_set_sync_interval(System::NTP::SyncIntervalMs); 
  sntp_init();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // needed to set timezone
}
void sendNtpPpacket(const char* address) {
  // see https://github.com/arduino-libraries/Ethernet/blob/master/examples/UdpNtpClient/UdpNtpClient.ino
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision

  udp.begin(System::NTP::LocalPort); // Use a local port for sending NTP requests
  udp.beginPacket(address, System::NTP::ServerPort);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
time_t getNtpTime() {
  // see https://github.com/arduino-libraries/Ethernet/blob/master/examples/UdpNtpClient/UdpNtpClient.ino
  sendNtpPpacket(System::NTP::ServerHost);

  int wait = 1000; // Max wait time in ms // TODO: make configurable
  int interval = 50;
  while (wait > 0) {
    delay(interval);
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      udp.read(packetBuffer, NTP_PACKET_SIZE);

      // Extract seconds since Jan 1 1900
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;

      const unsigned long seventyYears = 2208988800UL;
      time_t epoch = secsSince1900 - seventyYears;
      return epoch;
    }
    wait -= interval;
  }

  logger.warn(TAG_NTP_SYNC, "NTP request timed out after %d ms", 1000 - wait);
  return 0;
}
void checkNTP() { 
  // WARN: this function has side effects
  // WARN: this function was machine-generated
  struct timeval tv_before, tv_after;
  gettimeofday(&tv_before, nullptr);

  time_t systemTime = tv_before.tv_sec;
  ntpDiagnostics.lastDriftCheck = systemTime;
  time_t ntpTime = getNtpTime();

  gettimeofday(&tv_after, nullptr);

  if (ntpTime == 0) return;

  uint64_t sys_ms = ((uint64_t)(tv_before.tv_sec + tv_after.tv_sec) * 1000) / 2 + ((uint64_t)(tv_before.tv_usec + tv_after.tv_usec) / 2000);
  uint64_t ntp_ms = (uint64_t)ntpTime * 1000;
  int64_t drift_ms = (int64_t)(sys_ms - ntp_ms);

  ntpDiagnostics.lastDriftMs = drift_ms;

  int ntp_status = sntp_get_sync_status();

  logger.debug(TAG_NTP_DIAG,
               "NTP status: %d, NTP time: %010lu, drift: %ld ms",
               ntp_status,
               static_cast<unsigned long>(ntpTime),
               static_cast<long>(drift_ms));
}

int minutesSinceLastNtpSync() {
  time_t now = time(nullptr);
  if (ntpDiagnostics.lastNtpSync == 0) return -1;
  return (int)((now - ntpDiagnostics.lastNtpSync) / 60);
}
int secondsSinceLastNtpCheck() {
  time_t now = time(nullptr);
  if (ntpDiagnostics.lastDriftCheck == 0) return -1;
  return (int)(now - ntpDiagnostics.lastDriftCheck);
}
