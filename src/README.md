# Flashing guide

In order to flash the board from the *Arduino IDE*, the *Raspberry Pi Pico* SDK by *Earle F. Philhower* must be installed. To install the SDK, go to *Boards Manager* and search for *Raspberry Pi Pico*. The newest version should work. If not, Version 4.0.2 was used at the time of creatings document.

## Settings

For the Board, select: *Board:*->*Raspberry Pi Pico/RP2040*->*Generic RP2040*

|Parameter|Value|
|---|---|
|Boot Stage 2| W25Q128JV QSPI/4|
|Debug Level|None|
|Debug Port|None|
|C++ Exceptions|Disabled|
|Flash Size|16MB (Sketch: 8MB, FS: 8MB)|
|CPU Speed|133 MHz|
|IP/Bluetooth Stack|IPv4 Only|
|Optimize|Small (-Os) (standard)|
|RTTI|Disabled|
|Stack Protector|Disabled|
|Upload Method|Default (UF2)|
|USB Stack|Adafruit TinyUSB|

## Flash

Apply the settings above, and plug in the device. Select the new virtual com port and press upload. Once uploaded, the device will register as a unformated flash drive. Format the disk and once done, unplug the device. After plugging it back in again, the disk should register properly and a empty config file should be created. If this is nit the case, replug the device again.
