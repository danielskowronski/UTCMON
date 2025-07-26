# OLED - 256x64 with SSD1322

- Controller: SSD1322
- OLED Display 3.12" 256*64 25664 Dots Graphic LCD Module Display Screen LCM Screen SSD1322 Controller Support SPI
- https://www.aliexpress.com/item/1005007321062605.html
- datasheet of controller: https://www.displayfuture.com/Display/datasheet/controller/SSD1322.pdf
- comes configured as parallel 80XX, using here as "4SPI" so need to swap jumper from R6 to R5
- SPI connection from ESP32 works with frequencies up to 39.999 MHz

## PINS used

- 01: 3V3
- 02: GND
- 03: SCLK
- 05: SDI/MOSI
- 14: DC (data/command)
- 15: RESET
- 16: CS (chip select)
