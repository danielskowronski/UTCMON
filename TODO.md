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
  - [.] workaround for SPI transmission broken due to other cores triggering inetrrupts
  - [.] further SPI issue diagnostics
  - [ ] fix SPI transmission in u8g2  https://github.com/olikraus/u8g2/issues/2682

## current work

2025-07 - ...

- [x] controllable NTP sync
- [x] NTP drift detection
- [x] WiFi reconnect after ESP_ERR_WIFI_CONN reinits NTP client, which fails assertion `assert failed: sntp_setoperatingmode /IDF/components/lwip/lwip/src/apps/sntp/sntp.c:748 (Operating mode must not be set while SNTP client is running)` and crashes device
- [x] add icon to indicate wifi connection is lost
- [.] drawClock - all magic numers defined in UI.h as UIL/UIR
- [ ] rest of UI drawing - all magic numers defined in UI.h as UIL/UIR
- [ ] move TimeSync contents to some class
- [x] standardize debug messages on console
- [ ] improve code as of 2025-W30 (remove warnings, duplicates)
- [.] improve UI render formatting
- [ ] cover all places where we could check init status, attempt checking display update status 
- [-] better brightness control (without flicker) -> can't be done better
- [x] distance sensor - wrap in structs, fix logging and formatting
- [ ] light sensor - wrap in structs, fix logging and formatting
- [ ] timezone change UI - virtual buttons
- [ ] brightness control UI - virtual buttons
- [ ] watchdog for abnormal NTP sync -> reboot

## future work

- [ ] add logging to loki or similar system
- [ ] multithread operations??
- [ ] runtime control via WebAPI (timezone, timezones, display mode, brightness, schedules...)
- [ ] sensor status push via MQTT
- [ ] countdown mode
- [ ] multi-TZ mode
- [ ] simple text info pull from external server mode
