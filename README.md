# ESP32 Alarm Clock
This alarm clock was made using the Xiao ESP32C3 and the Xiao expansion board.
The project was built on the ESP-IDF and VSCode.

## Libraries used
* SSD1306/SH1106 driver for esp-idf: https://github.com/nopnop2002/esp-idf-ssd1306
* Rotary encoder driver for esp-idf: https://github.com/DavidAntliff/esp32-rotary-encoder

## Components used
* SEED Xiao Esp32C3
* SEED Xiao Expansion board
    * SSD1306 OLED I2C 128x64 display
    * PushButton (D1 on expansion board)
* Li-Po Battery (small enough to fit into the expansion board)

## Functionality implemented
* When the device powers on, it connects to local Wi-Fi
* The device gets the current time from an SNTP server 
* Alerts when there is an alarm
* Added a rotary encoder with a pushbutton for controlling the clock
* Added a menu to the state machine
    ### Caveats
    * Cannot set the time during runtime (SNTP gives the time)
    * Cannot set alarms during runtime (alarm times are hardcoded)
    * The menu does not work as the state machines are not set, yet
    * most caveats will resolve once save to EEPROM is implemented and appropriate state machines are designed

## Functionality to implement
* Save alarms to EEPROM
* Implement tones with a buzzer to alert the user in case of an alarm
* Implement MQTT to share data with a local Mosquitto server (already done, but no useful functionality yet)
