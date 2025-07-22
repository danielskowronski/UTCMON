# ToDo

internal reference: `DSx2500A`

## MVP

2025-03 - 2025-04

- [x] project design, parts planning and shopping
- [x] validate all parts and their libraries: dual displays, distance sensors, light sensors
- [x] plan circuits including pinouts, solder everything and test
- [x] implement prototype code
- [x] convert to platformio
- [x] initial clock drawing
- [x] initial NTP sync (+WiFi)
- [x] full planned clock drawing
- [x] commit hardware diagrams and docs
- [x] 3D case - base unit and displays
- [x] handle light sensors for brightness
- [x] debug display distance sensors
- [x] validate assembled unit

## first improvements

2025-04 - 2025-07

- [x] splash screen with debug info
- [x] stabilize distance sensors
- [.] stabilize display (long term tests)
  - [x] observation and research
  - [x] periodic screen redraw
  - [x] frequency and setup

## current work

2025-07 - ...

- [x] controllable NTP sync
- [x] NTP drift detection
- [ ] move TimeSync contents to some class
- [ ] standardize debug messages on console
- [ ] improve code as of 2025-W30 (remove warnings, duplicates)
- [ ] better brightness control (without flicker)
- [ ] timezone change UI - virtual buttons
- [ ] brightness control UI - virtual buttons
- [ ] watchdog for abnormal NTP sync -> reboot

## future work

- [ ] multithread operations??
- [ ] runtime control via WebAPI (timezone, timezones, display mode, brightness, schedules...)
- [ ] sensor status push via MQTT
- [ ] countdown mode
- [ ] multi-TZ mode
- [ ] simple text info pull from external server mode
