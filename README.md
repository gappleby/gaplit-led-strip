# GapLit led Strip Lights
Configure an ESP8266 module to control up to 8 virtual light segments in a single WS2812b LED Strip.

## Features

- Allows for the definition of up to 8 virtual lights.
- Each virtual light has a choice of transitions between on/off.
- Provides MQTT Control of each light.
- Uses a built-in webserver to allow configuration and control of an individual virtual light
- Compiles on standard Arduino compiler.
- All settings are configurable and persisted in flash.
- Firmware can be updated OTA via the web interface.

## Screen Shots
- [Home](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen1.png?raw=true)
![picture alt](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen1.png?raw=true "GapLit led Strip")
- [General Settings](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen2.png?raw=true) 
- [Light Settings](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen4.png?raw=true) 
- [MQTT Summary](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen5.png?raw=true)
- [Update Firmware](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen6.png?raw=true)

## Settings

### Login
The default login is "admin" with a blank password. This will be required for all settings' screens

### WiFi

This firmware turns into an access point on first load. Just connect to WiFi access point "gaplit-nnnn" and configure your WiFi settings.

After it reboots onto the network, use your router to find the DHCP assigned IP address.

### General Settings

After you have the ESP8266 connected to your network, you should 
- Select the "General Settings" menu
- Set the "Max Strip LEDs" to the maximum number of LEDs; and 
- Confirm the GPIO "Led Strip PIN Out" value 
  - See the "Tested Modules" example setting screens for tested values.
- Click the "Apply" button to set the values in memory
- Scroll down the bottom and click the "Save" button to commit the settings to persistent storage. 

This is a good time to reboot the ESP8266 to load these critical settings.

### Light Settings

A virtual lights need to be setup before you see anything on the LED strip.  Once setup, they can be used via the webui, but for MQTT, you need to reboot to reload the MQTT client configuration.

To set up a virtual light:
- Select Light Settings menu
- In Group "Light 1"
    - Set the MQTT Topic to 1 (even if you are not using MQTT yet) to enable the light.
    - Set the "Start Pixel" and "End Pixel" settings. See the "Light Settings" image above for sample values. 
- Click the "Apply" button to set the values in memory
- Scroll down the bottom and click the "Save" button to commit the settings to persistent storage. 

Click the Home menu option and you should now have an "LS Light 1" available to click on and off. No need to reboot yet as we have MQTT to configure.

### Set MQTT Settings

MQTT settings are configured under "General Settings".  You should have an MQTT broker already configure or use the one built in your automation server like Home Assistant.

- Select "General Settings" menu
- In Group "MQTT Settings"
    - Set the MQTT Enabled = 1
    - Set the MQTT Host = ip address of MQTT broker (or Home Assistant)
    - Set the MQTT User and MQTT Password. 
- Click the "Apply" button to set the values in memory
- Scroll down the bottom and click the "Save" button to commit the settings to persistent storage. 

Reboot to load these settings.  This can be done via the "Reboot" menu. (hint: click the reboot button)

### MQTT Summary

The MQTT Summary menu shows a complete summary of all the mqtt settings. 
Configure the same as you would for Tasmota firmware, but with the states and topics as displayed on this page.  

Refer example Home Assistant MQTT configuration file [lights.yaml](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/lights.yaml?raw=true) on how these values can be used. 

### Other settings

There are other subtle settings that I will eventually document.
Try out the global Tracer Effect by settings "Tracer LEDs" value to 3 and apply. 
Change the "Density" of a virtual light and power the light on / off (might be better to turn off the light first). 
Set an "Off Color" for the virtual light. 

Try out different colours for the virtual light. Use overlapping the start and end light LEDs of different virtual lights.

There is also a couple of other URLs not available in the menus.  
- http://{deviceIpAddress}/erasesettings 
  Wait 1 minutes before powering off/on.
- http://{deviceIpAddress}/stats 
  Returns a list of values of ESP8266 stats.


## Possible Applications

- Over bench lighting
 Install below overhead cupboards. Why just have one light from a strip of LEDs when you can have "zones" of light?

- Bedroom Lights
 Install around, under, over beds to have various glows areas with different colors. 
 In a kids room you can install around the room as a main light then change to a night light for when they are asleep. 
 You can even define multiple virtual lights for the same LEDs, and even define an off colour for a virtual light.

- Hallway lights
 Have the hallway lights be broken into a start, middle and end virtual lights on the same strip. 
 Change the colour by defining the same LEDs as different a virtual light and then turn it on overnight. e.g. a dim red glow is kinder on the eyes at 3am than the white light.

## Tested ESP8266 Boards	

Technically any ESP8266 board that will run Sonoff-Tasmota will run this firmware.

* Full > [Download latest compiled version](https://raw.githubusercontent.com/gappleby/gaplit-led-strip/master/releases/stable/gaplit-led-strip.bin)
* Minimal > [Download latest compiled version](https://raw.githubusercontent.com/gappleby/gaplit-led-strip/master/releases/stable/gaplit-led-strip-minimal.bin)

This project was initially built for the Wemos D1 R2 board to drive a WS2812B LED Strip to provide a set of virtual lights. By default it uses PIN D1 as a signal PIN for the WS2812B.  
Set the GPIO configuration to look like these in the [General Settings](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen2-wemos.png?raw=true).

It can also be configured to be used with a Sonoff Basic (and POW/Dual/etc) where the spare GPIO 14 pin is used to control the LED Strip. 

With the Sonoff, you can utilise the relay to switch the LED  power supply on and off. This will save power when all the virtual lights are off. 
Set the GPIO configuration to look like these in the [General Settings](https://github.com/gappleby/gaplit-led-strip/blob/master/releases/pics/screen2-sonoff.png?raw=true).

## LED Strip

This has been tested with a 300 LED Strip based on WS2812b. As it uses the NeoPixel library, it wouldn't be hard to support other multiple color strips

## Libraries

The project uses a variety of libraries, with a preference to Async based libraries for all network activities.

- Async WebServer https://github.com/me-no-dev/ESPAsyncWebServer
- Async WiFi Manager https://github.com/alanswx/ESPAsyncWiFiManager
- Async Mqtt Client https://github.com/marvinroger/async-mqtt-client
- Adafruit NeoPixel https://github.com/adafruit/Adafruit_NeoPixel

## Compiling

1. Download the source and load in Arduino studio.
2. Choose the right board type through "Tools">"Board">the type of board you have.
3. Download the above libraries using "Sketch" -> "Include Library" -> "Manage Libraries" and download Async WebServer, Async WiFi Manager, AsyncMqttClient, and NeoPixel.
4. Compile and upload to the connected ESP8266 module.
5. Build an image for OTA using "Sketch" -> "Export compiled binary".

When in doubt - follow the advice on the [Sonoff-Tasmota](https://github.com/arendst/Sonoff-Tasmota).

### ATTENTION All versions

Only Flash Mode DOUT is supported. Do not use Flash Mode DIO / QIO / QOUT as it might seem to brick your device.

See [Sonoff-Tasmota Wiki](https://github.com/arendst/Sonoff-Tasmota/wiki/Theo's-Tasmota-Tips) for background information on compilation issues.

## Credit

- [Sonoff-Tasmota](https://github.com/arendst/Sonoff-Tasmota) - the default firmware I use in all my Sonoff devices. 
  I actually loaded Tasmota first (using SonOTA), then replaced it with this firmware.  
  GapLit led Strip project started as a branch of the Sonoff-Tasmota project, then I realised I would break too much of Theo Arends fine work. 
  Though I did reuse some of his settings code, unfortunately the settings layout is no longer compatible with his settings' offsets.

## Future Features

- Languages
 Chinese will be next. I've prepared by isolating much of the UI to a separate language file that may need to be configured at compile time due to space considerations.

- Timers
 Add timers to each light to allow it to turn on/off at set times.  But use HomeAssistant if you really want to have true automation.

- Fancy Transitions
 Add individual transitions to each of the virtual light. This project was not intended to be flashy and bright - but it would be nice to have a slide in / out transition effect for on / off operations.

- Animated On
 Not happening soon as it could get tacky. GapLit led Strip project was to provide a set of light used every day - not festival lights.

## Version History

- Version 1.0 - Initial Version - WebServer, MQTT, FastLED. Used all  synchronous based libraries.  Had issues with more than 6 virtual lights.

- Version 2.0 - Async Version - Async WebServer, Async MQTT, NeoPixel. Async libraries solved most of the network limitations - and found to be more responsive. 
  NeoPixel introduced as it allowed for a configurable LED strip PIN. Added better icons for the virtual lights.

- Version 3.0 - Light Transition - Async WebServer, Async MQTT, NeoPixel. Added transitions to each of the light segments.  Refactored the code to contain the logic for relay and light segments into their own objects.

- Version 3.0.2 - PlatformIO - Added support for PlatformIO builds in addition to the Arduino.

- Version 3.0.3 - Minor fix - Fixed RGB order in config.

- Version 3.0.4 - Auth Fix - fixed the login code for MQTT.  Updated to support latest PlatformIO.  Refreshed external libraries

### Issues

- The tracer logic has not been re-implemented.

## License
This program is licensed under Lesser GPL-3.0.
