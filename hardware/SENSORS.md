# Sensors

- all current sensors use I2C
- each display board (left and right) use separate I2C bus as one sensor (VL53L0X) has no hardware control; moreover two some separate lines for displays are needed anyway and ESP32 has two I2C

Buses:

- LEFT:
  - SDA: 21
  - SCK: 22
- RIGHT:
  - SDA: 32
  - SCK: 33

## Light sensor - TSL2561

- 2x TSL2561 - one per board
- I2C address 0x39 on both
- oriented towards top of unit
- used to set display brightness by averaging lux value and adjusting to 0..255 arnge

## Luminosity sensor - VL53L0X

- 2x VL53L0X - one per board
- I2C address 0x52 on both
- oriented towards front of unit
- used as touchless buttons (to avoid pushing displays from monitor) and primitive presence sensor
