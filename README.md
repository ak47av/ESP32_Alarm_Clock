# ESP32 Alarm Clock
This alarm clock was made using the Xiao ESP32C3 and the Xiao expansion board.
The project was built on the ESP-IDF and VSCode.

## Libraries used
* SSD1306/SH1106 driver for esp-idf: https://github.com/nopnop2002/esp-idf-ssd1306

## Components used
* SEED Xiao Esp32C3
* SEED Xiao Expansion board
    * SSD1306 OLED I2C 128x64 display
    * PushButton (D1 on expansion board)
* Li-Po Battery (small enough to fit into the expansion board)

## Functionality implemented
* Shows the time since the device powered on
* Alerts when there is an alarm
    ### Caveats
    * Cannot set the time during runtime
    * Cannot set alarms during runtime (alarm times are hardcoded)
