```
export PROJBASE
```

1. clone https://github.com/espressif/esp32-arduino-lib-builder
2. without venv: `./install.sh`
3. `./build.sh -t esp3`

/Users/daniel/projects/UTCMON/espressif32/esp32-arduino-lib-builder/out/tools/esp32-arduino-libs/esp32/lib/libfreertos.a
/Users/daniel/.platformio/packages/framework-arduinoespressif32-libs/esp32/lib/libfreertos.a

cp /Users/daniel/projects/UTCMON/espressif32/esp32-arduino-lib-builder/build/esp-idf/freertos/libfreertos.a /Users/daniel/.platformio/packages/framework-arduinoespressif32-libs/esp32/lib/libfreertos.a
daae2e51d88c1c90440bbd24f5b0fb6cbd068aa29a2b764760a4bd04f360f68f  /Users/daniel/.platformio/packages/framework-arduinoespressif32-libs/esp32/lib/libfreertos.a
669c28799b5f1fc2cb5d08103088fd04c349c8a18e7286cf99dab9cc18b86ff3  /Users/daniel/.platformio/packages/framework-arduinoespressif32-libs/esp32/lib/libfreertos.a.orig


pio run --target clean
pio run --target uplaod && pio device monitor
