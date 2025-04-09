# UTCMON

Dual-screen clock intended designed mounting on top of *mon*itor (with USB camera in between), intended for displaying *UTC* and local time with proper NTP synchronization.

Project is baed on ESP32 for networking and 3.12" 256x64 OLED displays to match height of Logitech C920 camera. It'll additionaly include 3D printed cases for displays and central unit, light sensor for brightness, minimalistic physical controls and APIs for automation.

---

## Terminology, general ideas and assumptions

- Entire device consists of three units - 2 displays (left and right, both housing identical set of sensors, one large display and connector) and central unit (with ESP32 dev board, connectors and USB-C cable for power, debug and flashing).
- Display units are symmetrical in Y axis, intended to be placed in one line on top of computer monitor, separated by webcam, which is not part of the project. Connectors should be on inner edge of display units.
- This is a one-off, so standards are ajusted for prototype. No proper PCB is planned at this time.
