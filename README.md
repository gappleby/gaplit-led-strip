# GapLit Led Strip Lights
Configure an ESP8266 module to control up to 8 virtual light segments in a single WS2812b LED Strip.

## Features

- Allows for the defining of up to 8 virtual light segments.
- Each light segment has a soft on/off behaviour.
- Provides MQTT Control of each light.
- Uses a build in webserver to allow configuration and control the light segments.
- Compiles on standard Arduino compiler.
- All settings are configurable and persisted in flash.
- Firmware can be updated OTA via the web interface.

## Screen Shots
- [Home](http://www.gappleby.com/light/GapLit/screen1.png)
 ![picture alt](http://www.gappleby.com/light/GapLit/screen1.png "GapLit LED Strip")
- [General Settings](http://www.gappleby.com/light/GapLit/screen2.png) 
- [Light Settings](http://www.gappleby.com/light/GapLit/screen4.png) 
- [MQTT Summary](http://www.gappleby.com/light/GapLit/screen5.png) 
- [Update Firmware](http://www.gappleby.com/light/GapLit/screen6.png) 

## Settings

### Login
The default login is "admin" with a blank password. This will be required for anything that can change settings.

### Wifi

This firmware turns into an access point on first load. Just connect to wifi access point "gaplit-ABCD" and configure your wifi settings. After it reboots onto the network, use your router to find the DHCP assigned IP address.

### General Settings

After you have the esp8266 connected to your network you should set the "Max Strip LEDs" and confirm the GPIO "Led Strip PIN Out" value. Click the "Apply" button then scroll down the bottom and click the "Save" button to commit the settings to persistent storage. This is a good time to reboot the ESP8266 to load these critical settings.

### Light Settings

Choose menu Light Settings, then use Light 1 to start with and configure the MQTT topic (even if you are not using MQTT yet) and set it to a small positive number, i.e. 1. Next set the Start and End Pixel. See the Light Settings image above as an example set of values. Apply, then Save. Click the Home menu option and you should now have a "LS Light 1" available to click on and off. No need to reboot.

### Set MQTT Settings

MQTT settings are configure under the "General Settings". Make sure you configure at least the MQTT Enabled = 1, MQTT Host = ip address of MQTT broker or HASS server, MQTT User and Password. Apply, then Save. Rebooting to load these ssettings.

### MQTT Summary

The MQTT Summary menu shows a complete summary of all the mqtt settings. Configure the same as you would for tasmota firmware, but with the states and topics are displayed on this page.  

See example Home Assistant MQTT configuration file [lights.yaml](https://www.gappleby,com/light/GapLit/lights.yaml) on how these values can be used. 

### Other settings

There are subtle little settings that I will eventually document. Try out the global Tracer Effect by settings "Tracer LEDs" value to 3 and apply. Change the "Density" of a light segment and power the light on / off (might be better to turn off the light first). Set an Off Color for the light segment. Try out different color for the light segments. Use overlapping the start and end light LEDs of different Light segments.

## Applications

- Over bench lighting
  Install below overhead cupboards. Why just have one light from a strip of LEDs when you can have "zones" of light?

- Bedroom Lights
  Install around, under, over beds to have various glows areas with different colors. In a kids room you can install around the room as a main light then as a night light for when they are asleep. You can even define multiple light segments for the same leds, and even define an off color for a light segment.

- Hallway lights
  Have the hallway lights be broken into a start, middle and end on the same strip. Change the color by defining the same leds as a different light segment and turn that on overnight. e.g. a dim red glow is kinder on the eyes at 3am than the white light.

## Tested ESP8266 Boards
Technically any ESP8266 board that will run sonoff-tasmota will run this firmware.

* Wemos D1 R2 > [Download latest compiled version](https://raw.githubusercontent.com/gappleby/gaplit-led-strip/master/releases/stable/gaplit-led-strip.ino.d1_mini.bin)
  This project was initially built for the Wemos D1 R2 board to drive a WS2812B LED Strip to provide a set of virtual lights. By default it uses PIN D1 as a signal PIN for the WS2812B.  Set the GPIO configuration to look like these in the [General Settings](http://www.gappleby.com/light/GapLit/screen2-wemos.png).
 
* Sonoff > [Download latest compiled version](https://raw.githubusercontent.com/gappleby/gaplit-led-strip/master/releases/stable/gaplit-led-strip.ino.generic.bin)
  It can also be configured to be used with a Sonoff Basic (and POW/Dual/etc) where the spare GPIO 14 pin is used to control the LED Strip. With the Sonoff, you can also the relay to power on and off to control a power supply to the LED Strip. This will save power when all the LED light segments are off. Set the GPIO configuration to look like these in the [General Settings](http://www.gappleby.com/light/GapLit/screen2-sonoff.png).

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
2. Choose the right board type under "Tools".
3. Download the above libraries using "Sketch" -> "Include Library" -> "Manage Libraries" and download Async WebServer, Async Wifi Manager, AsyncMqttClient, and NeoPixel.
4. Compile and upload to the ESP8266 module connected.
5. Built an image for OTA using "Sketch" -> "Export compiled binary".

When in doubt - follow the advice on the [Sonoff-Tasmota](https://github.com/arendst/Sonoff-Tasmota).

### ATTENTION All versions

Only Flash Mode DOUT is supported. Do not use Flash Mode DIO / QIO / QOUT as it might seem to brick your device.

See [Wiki](https://github.com/arendst/Sonoff-Tasmota/wiki/Theo's-Tasmota-Tips) for background information.

## Credit

- [Sonoff-Tasmota](https://github.com/arendst/Sonoff-Tasmota)
  Simply the default firmware I use in all my Sonoff devices. I actually load tasmota first (using SonOTA), then replaced it with this firmware.  GapLit Led Strip project started as a branch of the Sonoff-Tasmota project, then I realised I would break too much of Theo Arends fine work. I did reuse some of his settings code originally, but unfortunately it is no longer compatible with his settings offsets.

## Future Features

- Languages
  Chinese will be next. I've prepared by isolating much of the UI to a separate language file that may need to be configured at compile time due to space considerations.

- Timers
  Add timers to each light to allow it to turn on/off at set times.  But use HomeAssistant if you really want to have true automation.

- Fancy Transitions
  Add individual transitions to each of the light segments. This project was not intended to be flashy and bright - but it would be nice to have a slide in / out transition effect for on / off operations.

- Animated On
  Not happening soon as it could get tacky. GapLit Led Strip project was to provide a set of light used every day - not festival lights.

## Version History

- Version 1.0 - Initial Version - WebServer, MQTT, FastLED. All sync libraries and had issues with more than 6 light segments as the 4 TCP queue depth locked up under any slight load.

- Version 2.0 - Async Version - Async WebServer, Async MQTT, NeoPixel. Async libraries solved most of the network limitations - and so more responsive. NeoPixel introduced as it allowed for the signal PIN to be configurable. Added better icons for the light segments.

### Issues

- There appears to be a connection limit on the webserver that no matter how much async I do.  If your HTML page doesn't load correct the first time, refresh and the site will load as all the resource files will be loaded from cache.

## License
This program is licensed under Lesser GPL-3.0.
