# UWU Firmware

The Universal Wordtyping Unit (UWU) firmware is a firmware for RP2040 based USB input devices.\
It works by parsing a settings file ```Layout.txt``` located on the internal Flash storage of the RP2040 which is exposed via the USB Mass Storage class. The language used for configuration is the AniMacro scripting language, a set of commands specially designed for use with the UWU firmware.

## Filesystem structure

To store configuration files, the UWU firmware allocates a portion of the micro controller’s flash storage to a PC-mountable filesystem. This allows the user to access the unit as a flash drive. Located on the drive is a configuration file that enables the user to configure the unit with the AniMacro scripting language.\
To enable this functionality, two primary challenges must be addressed:

1. Ensure that flash storage is accessible and available within the firmware.
2. Pass the allocated storage to the PC via USB as a mountable drive.

The first challenge is resolved through the utilisation of the Raspberry Pi Pico Arduino core in conjunction with the incorporated Adafruit SPI Flash library. The Arduino core enables the partial reservation of the microcontroller’s system flash for use from within the program. The SPI flash library was built to make flash storage connected to the microcontroller via the SPI bus accessible from within the program. It also offers built-in support for the FAT filesystem via the included SDFAT library. Due to the RP2040’s flash being connected externally via QSPI (Quad SPI), the SPI flash library accepts a reference from the Arduino core to allocate some of the system flash as file storage. The second challenge is resolved using the TinyUSB implementation from Adafruit. The TinyUSB library facilitates the registration of the microcontroller over USB as a mass storage device. This mass storage class incorporates callback functions to exchange data between the flash storage and the computer,enabling bi-directional communication. For the data to be useable by the microcontroller and PC, the flash storage is formatted as a FAT filesystem and mapped to an Adafruit SPI Flash library SDFAT object on boot. The microcontroller then parses the configuration file located in the filesystem and maps it into the ’Module structure’.

## AniMacro scripting language

The 'AniMacro' scripting language allows the user to configure any Hardware Interface running the 'UWU firmware'.\
Configurations written in AniMacro are automatically loaded upon start-up and subsequently parsed using regular expressions. Additionally, the firmware can detect changes in the configuration file, facilitating its reload as needed, thereby enabling dynamic configuration adjustments at runtime.

A single configuration file accommodates configurations for various hardware UWU modules. The firmware selectively parses configurations pertinent to the currently connected hardware modules. This design choice allows for the seamless connection of multiple modules during runtime.

### Syntax

The 'AniMacro' syntax is based loosely on LaTeX syntax. A configuration for a specific module is wrapped in a ```begin``` and ```end``` qualifier:

```c
begin{module}
...
end{module}
```

Within a module configuration, multiple layers can be incorporated. Layers are a mechanism to implement diverse button configurations, facilitating dynamic switching between them at runtime. Layers can be added by typing ```Layer``` followed by the number of the layer:

```c
begin{module}
Layer x
...
end{module}
```

> [!IMPORTANT]
>By default the Firmware starts on layer 1. In the absence of a configured layer 1, the firmware remains inactive. It is advisable to consistently include a layer 1 configuration to ensure proper functionality.

All devices responsible for executing actions within a layer are designated using the ```Button``` keyword. Whether these devices are faders, potentiometers, or conventional buttons, they are uniformly referenced with the ```Button``` keyword.\
Buttons are implemented using the 'Button' keyword, followed by the button number, a colon, and a space. Subsequently, functionality can be added to the button by incorporating commands within the same line as the 'Button' keyword:

```c
begin{module}
Layer x
    Button a: commands
    Button b: commands
...
end{module}
```

Commands are categorized into several groups, which can be combined within a button configuration:

- Plain text:\
Entering plain text (e.g., "this is text123") utilizes the keyboard HID interface to type out the provided text.
    > [!NOTE]
    > Only ASCII characters are accepted in plain text. Non-ASCII characters (e.g. ```ä```) are managed with separate commands.
- ```\COMMAND``` with no arguments:\
    These commands serve as non-configurable placeholders for specific actions. For instance, pressing the 'Enter' key on the keyboard (```\ENTER_KEY```) or adding umlauts (```\ae``` => ```ä```).
- ```\COMMAND{}``` with arguments:\
    These commands are commonly employed to configure actions and callbacks within the firmware.\
    For example: ```\COLOR{const,green}``` would set the button to a constant green colour.\
    ```\MIDI_CC_KEY{16,1}\COLOR{midi,white}``` would configure a MIDI callback, binding the button to MIDI CC number 16 on MIDI channel 1. When the button is triggered, a MIDI message is dispatched to the host (PC), initiating an action. Subsequently, the host can relay back the state of the triggered action. The response from the host is then linked to the button colour, with a response greater than 126 denoting a triggered colour override, and a response of less than 126 signifying a reversion back to the layer default.

### List of commands

#### AniMacro commands

| Command | Description | Note |
| --- | --- | --- |
| Plain text | Types out the given text via the HID Interface | Only works with ASCII characters|
| ```\ae``` ```\oe``` ```\ue``` | Types out ```Umlauts``` (ä,ö,ü) | |
| ```\SZ_KEY``` | Sends keycode of ```ß``` key | |
| ```\GRAVE_KEY``` | Sends keycode of ```backtick``` ('´') key | |
| ```\CAPS_LOCK_KEY``` | Sends keycode of ```caps lock``` key | |
| ```\LSHIFT_KEY``` | Sends keycode of ```left shift``` key | |
| ```\RSHIFT_KEY``` | Sends keycode of ```right shift``` key | |
| ```\LCTRL_KEY``` | Sends keycode of ```left control``` key | For control combinations (e.g. ```CTRL``` + ```C```) refer to ```\STRG{char}```|
| ```\RCTRL_KEY``` | Sends keycode of ```right control``` key | For control combinations (e.g. ```CTRL``` + ```C```) refer to ```\STRG{char}```|
| ```\LATL_KEY``` | Sends keycode of ```left alt``` key | |
| ```\RATL_KEY```| Sends keycode of ```right alt``` key | |
| ```\WIN_KEY``` | Sends keycode of ```windows``` key | |
| ```\APP_KEY``` | Sends keycode of ```app``` key | |
| ```\RMETA_KEY``` | Sends keycode of ```right meta``` key | |
| ```\SPACE_KEY``` | Sends keycode of ```space bar``` key | Equivalent to typing space character in plain text |
| ```\BACKSPACE_KEY``` | Sends keycode of ```backspace``` key | |
| ```\DEL_KEY``` | Sends keycode of ```delete``` key | |
| ```ENTER_KEY``` | Sends keycode of ```enter``` key | |
| ```ESC_KEY``` | Sends keycode of ```escape``` key | |
| ```Tab_KEY``` | Sends keycode of ```tab``` key | |
| ```F[x]_KEY``` | Sends keycode of ```F[x]``` key | ```[x]``` to be replaced with number from ```1-12```|
| ```[arrow]_KEY``` | Sends keycode of ```[arrow]``` key | [arrow] to be replaced with one of: ```UP```, ```DOWN```, ```LEFT```, ```RIGHT```|
| ```[arrow]_KEY{[x]}```| Sends keycode of ```[arrow]``` key ```[x]``` times | ```[x]``` to be replaced with number and ```[arrow]``` to be replaced with one of: ```LEFT, RIGHT``` |
| ```\STRG{char}``` | Sends control combinations | Example: ```\STRG{c}``` => ```CTRL``` + ```C```|
| ```\MIDI_CC{[CT],[CH]}``` | Sends MIDI CC with number ```[CT]``` on MIDI channel ```[CH]``` with value from button | Replace ```[CT]``` with number from 0 to 127 and ```[CH]``` with number from 1-15. The button has to have a value or be a ```fader```/```potentiometer```/```encoder```.|
| ```\MIDI_CC_KEY{[CT],[CH]}``` | Sends MIDI CC with number ```[CT]``` on MIDI channel ```[CH]``` with value 127 | Replace ```[CT]``` with number from ```0``` to ```127``` and ```[CH]``` with number from ```1-15```. Used to trigger button events in software|
| ```\COLOR{[M],[C]}``` | Sets colour mode of button to ```[M]``` with colour ```[C]``` | Refer to ```colour mode list``` for valid mode ```[M]``` and ```colour list``` for valid colour ```[C]```|
| ```\COLOR{[M],[R],[G],[B]}``` | Sets colour mode of button to ```[M]``` with colour comprised of ```[R]```,```[G]``` and ```[B]``` | Refer to ```colour mode list``` for valid mode ```[M]```|
| ```\LAYER{[L]}``` | Sets active Layer to ```[L]``` | Layer ```[L]``` should be defined. Example: For ```\Layer{2}```, ```Layer 2``` should be defined.|

#### Colour mode list

|<div style="width:90px">Colour Mode</div> | Description | Note|
|:---|:---|:---|
| ```const``` | Sets a constant colour | |
| ```pressed``` | Sets the given colour if the button is triggered | |
| ```not pressed``` | Sets the given colour if the button is not triggered | |
| ```toggle``` | Toggles the colour on button trigger | |
| ```MIDI``` | Sets the colour based on message received over MIDI | Button has to be set as MIDI button with ```\MIDI_CC_KEY```. The channel and number set in ```\MIDI_CC_KEY``` will be used to trigger the colour|
| ```disabled``` | Disables the colour on the key | |

#### Colour list

|Colour|Note|
|:---|:---|
| ```red``` | |
| ```yellow``` | |
| ```green``` | |
|```cyan``` | |
| ```blue``` | |
| ```magenta``` | |
| ```white``` | |
| ```none/off``` | Used for modes, where colour is not a relevant parameter|
