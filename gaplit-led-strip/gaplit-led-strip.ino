/*
    This file is part of "GapLit Led Strip".

    "GapLit Led Strip" is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    "GapLit Led Strip" is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with "GapLit Led Strip".  If not, see <http://www.gnu.org/licenses/>.
*/

#include <core_version.h>
#include <list>
#if defined(ESP8266)
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#else
#include <WiFi.h>
#endif
#include <DNSServer.h>            // https://github.com/esp8266/Arduino
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPAsyncWiFiManager.h>  // https://github.com/alanswx/ESPAsyncWiFiManager

#include <Ticker.h>
#include <AsyncMqttClient.h>      // https://github.com/marvinroger/async-mqtt-client

#include <ESP8266mDNS.h>          // https://github.com/esp8266/Arduino

#include <Adafruit_NeoPixel.h>    // https://github.com/adafruit/Adafruit_NeoPixel

#include "settings.h"             // 
#include "localwebsite.h"         //

#include "lightsegment.h"         // LightSegment logic
#include "coloredlightsegment.h"  // Colored Light Segment with transition logic
#include "relaycontrol.h"         // Relay control

char host_name[33];
char mqtt_client[33];
char mqtt_topic[33];

#define REBOOT_DISABLED -1
int reboot_counter = REBOOT_DISABLED;

#define ERASE_FLASH_DISABLED -1
int erase_flash_counter = ERASE_FLASH_DISABLED;

// Relay On/Off states and registers
RelayControl relayControl;

// NeoPixel setup
std::list<std::shared_ptr<LightSegment>> lightSegments;
BlackWhiteLightSegment LIGHTSEGMENT_NULL;
Ticker updateLightSegmentsTimer;

Adafruit_NeoPixel stripLeds = Adafruit_NeoPixel();

const long STATUS_RESEND_PERIOD =  60000L;

// Prototypes
void mqttCallback(char* topic, char* payload, size_t length, size_t index, size_t total);
void transitionLeds();
void turnLightOn(int n);
void turnLightOff(int n);
void scheduleReboot();
int getLightTopic(int index);
String getLightDisplayName(int index);
bool getLightState(int index);

// This can be used to output the date the code was compiled
const char compile_date[] = __DATE__ " " __TIME__;

AsyncWebServer httpServer(80);
DNSServer dns;
LocalWebsite httpLocalWebsite;

// Wifi Manager will try to connect to the saved AP. If that fails, it will start up as an AP
// which you can connect to and setup the wifi
AsyncWiFiManager wifiManager(&httpServer, &dns);

// Wifi Client
WiFiClient wifiClient;

// Initialize MQTT
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker mqttResendTimer;

// Led Blink timer
Ticker blinkTimer;

// Debug to Serial Flag
bool debug_serial_output = false;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

Settings settings;

void setup() {
  // Start the serial line for debugging
  Serial.begin(115200);

  // Load the settings
  settings.load();
  settings.loadDelta();

  // Set the general debug level.  Specific level can be used if required.
  debug_serial_output = settings.getSerialLogLevel() > 0;

  // Don't bother to dump the settings normally.
  // Really just used to diagnose upgrade issues between
  // version where the config values have changed.
  if (debug_serial_output) {
    settings.dumpSettings();
  }

  // Setup Status LED
  if (settings.settings.status_light_gpio >= 0) pinMode(settings.settings.status_light_gpio, OUTPUT);
  blinkTimer.attach(0.5, blinkLedEvent);
  blinkLed(5, 100);

  // Configure settings
  settings.composeSetting(mqtt_client, settings.settings.mqtt_client, sizeof(mqtt_client));
  settings.composeSetting(mqtt_topic, settings.settings.mqtt_topic, sizeof(mqtt_topic));
  settings.getHostname(host_name, sizeof(host_name));

  // Setup LightStrip
  // stripLeds = Adafruit_NeoPixel(settings.settings.ls_pixels, settings.settings.ls_gpio, NEO_GRB + NEO_KHZ800);
  stripLeds.updateType(NEO_GRB + NEO_KHZ800);
  stripLeds.updateLength(settings.settings.ls_pixels);
  stripLeds.setPin(settings.settings.ls_gpio);
  stripLeds.begin();
  stripLeds.show();

  // Set the wifi host name.  Unless overridden composed using MAC address
  WiFi.hostname(host_name);

  initializeRelay();
  initializeLightSegments();

  // Set up the MQTT Client
  setupMqtt();

  // Set the wifi config portal to only show for 3 minutes, then continue.
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.autoConnect(host_name);

  // Setup the URLs
  httpLocalWebsite.setup(&httpServer, &settings);

  httpServer.begin();

  MDNS.begin(host_name);
  MDNS.addService("http", "tcp", 80);

  // Publish initial state
  mqttResendTimer.attach(STATUS_RESEND_PERIOD / 1000, publishLightStates);

  // Trigger the light segments
  updateLightSegmentsTimer.attach(0.02, transitionLeds);

}

void loop() {

  reconnectWifi();

  processBackgroundFlags();

}

void setupMqtt() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(settings.settings.mqtt_host, settings.settings.mqtt_port);
}

void initializeRelay() {
  relayControl.setStartDelayTime( settings.settings.relay_start_delay );
  relayControl.setEndDelayTime( settings.settings.relay_stop_delay );
  relayControl.setGpioPin( settings.settings.relay_gpio );
}

void initializeLightSegments() {

  // Load the configured light segments

  for (int n = 0; n < LIGHT_SEGMENTS_MAX ; n++)  {
    if (settings.settings.ls_topicIndex[n] > 0) {
      reloadLightSegment(n);
      if (settings.settings.ls_powerOnState[n]) {
        turnLightOn(n);
      }
    }
  }
}

void reloadLightSegment(int n) {

  if (settings.settings.ls_topicIndex[n] > 0) {
    std::shared_ptr<LightSegment> lightSegment;

    std::shared_ptr<ColoredLightSegment> colorLs(new ColoredLightSegment());
    colorLs->setPixelOnColor( { settings.settings.ls_colourOn[n][0], settings.settings.ls_colourOn[n][1], settings.settings.ls_colourOn[n][2]} );
    colorLs->setPixelOffColor( { settings.settings.ls_colourOff[n][0], settings.settings.ls_colourOff[n][1], settings.settings.ls_colourOff[n][2]} );
    colorLs->setTransitionType(settings.settings.ls_transition[n]);
    lightSegment = colorLs;

    lightSegment
    ->setState(INIT)
    ->setIndex(n)
    ->setStartEndPixel(settings.settings.ls_startPixel[n], settings.settings.ls_endPixel[n])
    ->setMqttId(settings.settings.ls_topicIndex[n])
    ->setDensity(settings.settings.ls_topicIndex[n])
    ->setSerialDebug(false);

    lightSegment->setLightState(false);
    lightSegment->reset();
    lightSegments.push_back(lightSegment);
  }

  // Remove the old entry for that setting index
  for (auto ls : lightSegments) {
    if (ls->getIndex() == n && ls->isActive()) {
      ls->setState(RETIRED);
    }
  }

  // Active the new entry for that setting index
  for (auto ls : lightSegments) {
    if (ls->getIndex() == n && ls->getState() == INIT) {
      ls->setState(ACTIVE);
    }
  }

}


/**
   LED soft on/off logic (and tracer effect)
*/
#define TRANSITION_DELAY 50L
int last_tracerPixel = -1;

int blendValue(int original, int target)
{
  int c = original; int d = target;
  if (c > d) {
    c = (c + d - 1) / 2;
    if (c < d) c = d;
  }
  else if (c < d) {
    c = (5 * c + d + 1) / 6;
    if (c > d) c = d;
  }
  return c;
}

void transitionLeds()
{

  // Update the relay state
  relayControl.update();

  if (!relayControl.isPowerSteady())  {
    if (settings.getSerialLogLevel() > 2) Serial.printf("\nRelay : Power OFF");
    return;
  }

  if (settings.getSerialLogLevel() > 2) Serial.printf("\nRelay : Power ON");


  bool updateRequired = false;

  for (auto lightSegment : lightSegments) {

    if (lightSegment->isActive()) {

      if (lightSegment->update()) {

        // Copy the current Pixel value from the light segment to the light strip
        CRGB pixel;
        int pixels = lightSegment->getNumPixels();
        int startPixel = lightSegment->getStartPixel();
        for (int m = 0; m < pixels; m += lightSegment->getDensity()) {
          pixel = lightSegment->getPixel(m);
          stripLeds.setPixelColor(startPixel + m, stripLeds.Color(pixel.r, pixel.g, pixel.b));
        }

        if (pixels > 0) {
          updateRequired = true;
        }

      }
    }
  }

  if (updateRequired) {
    stripLeds.show();
  }

  /*
     // Need to add the tracer logic back again.
     // But the new logic will need to read the pixel from the strip

     // Set the non-specific light tracer
     if (settings.settings.ls_tracerPixels > 0) {

       last_tracerPixel += 1;

       if (last_tracerPixel >= settings.settings.ls_pixels || last_tracerPixel < 0) {
         last_tracerPixel = 0;
       }

       int m = settings.settings.ls_tracerPixels;
       for (int n = last_tracerPixel; n < settings.settings.ls_pixels && m-- > 0; n++)
       {
         if (!(targetLeds[n].r == 0 && targetLeds[n].b == 0 && targetLeds[n].g == 0))
         {
           leds[n].r = settings.settings.ls_tracerColour[0];
           leds[n].g = settings.settings.ls_tracerColour[1];
           leds[n].b = settings.settings.ls_tracerColour[2];
         }
       }
     }

  */

  // Clean out any expired / retired light segment objects
  for (auto lightSegment = lightSegments.begin(); lightSegment != lightSegments.end(); ++lightSegment) {
    // Clean up
    if ((*lightSegment)->getState() == RETIRED) {
      lightSegments.erase(lightSegment);
    }
  }





}

// Use for all things that should be excuted regularly
void processBackgroundFlags() {
  if (reboot_counter != REBOOT_DISABLED)
  {
    if (reboot_counter == 0)
    {
      ESP.restart();
      delay(1000);
    }

    reboot_counter -= 1;
    delay(50);
  }

  if (erase_flash_counter != ERASE_FLASH_DISABLED)
  {
    if (erase_flash_counter == 0)
    {
      // Erase all settings
      settings.eraseAll(true);
      delay(1000);

      // reboot on next loop
      reboot_counter = 1;
    }

    erase_flash_counter -= 1;
    delay(50);
  }

}

/**
   MQTT Status Logic
*/

//
// Get the current light state
bool getLightState(int index) {

  for (auto &lightSegment : lightSegments) {
    if (lightSegment->isActive() && lightSegment->getIndex() == index) {
      return lightSegment->getLightState();
    }
  }

  return false;

}

std::shared_ptr<LightSegment> getLightSegment(int index) {

  for (auto &lightSegment : lightSegments) {
    if (lightSegment->isActive() && lightSegment->getIndex() == index) {
      return lightSegment;

    }
  }

  // Create a NULL LightSegment to return nothing.
  std::shared_ptr<LightSegment> nullLs(new BlackWhiteLightSegment());
  return nullLs;
}

void setLightState(int index, bool isOn) {

  for (auto lightSegment : lightSegments) {
    if (lightSegment->isActive() && lightSegment->getIndex() == index) {
      lightSegment->setLightState(isOn);
      if (debug_serial_output) Serial.printf("\nLightSegment %d : %s", index, isOn ? "ON" : "OFF");
      return;
    }
  }

}

//
// Publish Light State
void publishLightState(int index)
{
  if (!settings.settings.mqtt_enabled) {
    return;
  }

  std::shared_ptr<LightSegment> lightSegment = getLightSegment(index);
  int mqttTopicId = lightSegment->getMqttId();
  bool currentLightState = lightSegment->getLightState();

  // If enabled
  if (mqttTopicId > 0) {

    // Publish MQTT

    char topic[120];
    String suffix = String(MQTT_SUFFIX);
    suffix += String(mqttTopicId);
    settings.composeMqttTopic(topic, sizeof(topic), 1, mqtt_topic, suffix.c_str());

    uint16_t packetId = mqttClient.publish(topic, 1, true, currentLightState ? settings.settings.mqtt_state_text[1] : settings.settings.mqtt_state_text[0]);

    if (debug_serial_output) Serial.printf("\nMQTT Publish : Topic=%s , State=%s packetId=%d", topic, currentLightState ? settings.settings.mqtt_state_text[1] : settings.settings.mqtt_state_text[0], packetId);
  }
}

void publishLightStates()
{
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    publishLightState(n);
  }

  if (settings.settings.mqtt_enabled) {
    if (debug_serial_output) Serial.printf("\nMQTT Publish : All topics resent");
  }
}

int blinks = 0;
void blinkLedEvent()
{
  if (blinks < 0) return;
  int currentState = LOW;
  if (settings.settings.status_light_gpio >= 0) currentState = digitalRead(settings.settings.status_light_gpio);
  if (currentState == LOW)
  {
    if (debug_serial_output) Serial.printf("\nBlink LED : Off");
    blinks--;
    if (settings.settings.status_light_gpio >= 0) digitalWrite(settings.settings.status_light_gpio, HIGH);
  }
  else
  {
    if (debug_serial_output) Serial.printf("\nBlink LED : On  Count = %d", blinks + 1);
    if (settings.settings.status_light_gpio >= 0) digitalWrite(settings.settings.status_light_gpio, LOW);
  }
}

//
// Blink the led the number multiple times.
//
void blinkLed(int blinkCount, int period)
{
  blinks = blinkCount;
}




void setRelayStateIfNoLightSegmentsOn(bool relayState) {

  bool anyLightsOn = false;
  for (auto lightSegment : lightSegments) {
    if (lightSegment->isActive() && lightSegment->getLightState()) {
      anyLightsOn = true;
    }
  }
  if (!anyLightsOn) {
    if (relayState) {
      relayControl.turnOn();
    } else {
      relayControl.turnOff();
    }
  }
}
/**
   Virtual Light Segment control
*/
void turnLightOn(int n)
{
  // If all the lights off, turn on the relay
  setRelayStateIfNoLightSegmentsOn(true);

  std::shared_ptr<LightSegment> ls = getLightSegment(n);
  if (!ls->isActive()) return;

  setLightState(n, true);

  publishLightState(n);
}

void turnLightOff(int n)
{
  std::shared_ptr<LightSegment> ls = getLightSegment(n);
  if (!ls->isActive()) return;

  setLightState(n, false);

  publishLightState(n);

  // If all the lights off, turn off the relay
  setRelayStateIfNoLightSegmentsOn(false);
}


/**
   MQTT Logic
*/

// MQTT callback function -(Use only if topics are being subscribed to)
void mqttCallback(char* topic, char* payload, size_t length, size_t index, size_t total) {

  if (debug_serial_output) Serial.printf("\nMQTT Command : Topic = %s, Payload length = %d", topic, length);

  if (length == 0) {
    return;
  }

  // Convert topic to string to make it easier to work with
  String strTopic = String((char*)topic);

  // Extract the payload as a string
  byte data[120];
  memcpy(data, payload + index, length);
  data[length] = 0x00;

  int updatedLeds = 0;

  // Handle TOPIC_LIGHT_POWER_PREFIX
  for (auto lightSegment : lightSegments) {

    if (lightSegment->isActive()) {

      char targetTopic[120];
      String suffix = String(MQTT_SUFFIX);
      suffix += String(lightSegment->getMqttId());
      settings.composeMqttTopic(targetTopic, sizeof(targetTopic), 0, mqtt_topic, suffix.c_str());

      if (strTopic == targetTopic) {

        updatedLeds += 1;

        String strValue = String((char*)data);

        if (debug_serial_output) Serial.printf("\nMQTT Command : Topic = %s, Value = %s", strTopic.c_str(), strValue.c_str());

        if (strValue == settings.settings.mqtt_state_text[1]) {
          turnLightOn(lightSegment->getIndex());
        }

        if (strValue == settings.settings.mqtt_state_text[0]) {
          turnLightOff(lightSegment->getIndex());
        }

        publishLightState(lightSegment->getIndex());
      }
    }
  }

}

void resubscribeTopics() {

  if (!settings.settings.mqtt_enabled) {
    return;
  }

  for (auto lightSegment : lightSegments) {

    if (lightSegment->isActive()) {

      if (lightSegment->getMqttId() > 0) {

        char targetTopic[120];
        String suffix = String(MQTT_SUFFIX);
        suffix += String(lightSegment->getMqttId());
        settings.composeMqttTopic(targetTopic, sizeof(targetTopic), 0, mqtt_topic, suffix.c_str());

        uint16_t packetIdSub = mqttClient.subscribe(targetTopic, 2);
        if (debug_serial_output) Serial.printf("\nMQTT Subscribe : Topic = %s packetId = %d", targetTopic, packetIdSub);

      }
    }
  }
}


/**
   Wifi Client Logic
*/

void connectToMqtt() {

  if (settings.settings.mqtt_enabled) {
    if (debug_serial_output) Serial.printf("\nMQTT Connect : Attempting\n");
    mqttClient.connect();
  }

}

void reconnectWifi() {

  if ( WiFi.status() == WL_CONNECTED ) {
    return;
  }

  if (debug_serial_output) Serial.printf("\nWifi : Not connected - trying autoConnect with hostname %s\n", host_name);
  wifiManager.autoConnect(host_name);

  // Connect to MQTT
  if ( WiFi.status() != WL_CONNECTED ) {

    if (debug_serial_output) Serial.printf("\nWifi : Failed to connect Wifi, try again in 5 seconds");

  }

}

/**
   Schedule the reboot cycles - based on 100ms cycles
*/
void scheduleReboot() {
  reboot_counter = 50;
}

/**
   Schedule the reboot cycles - based on 100ms cycles
*/
void scheduleEraseAllSettings() {
  erase_flash_counter = 50;
}



/**
   MQTT Client Events
*/

void onWifiDisconnect(const WiFiEventStationModeDisconnected & event) {

  if (settings.settings.mqtt_enabled) {
    if (debug_serial_output) Serial.printf("\nMQTT Disconnected : Lost connection");
    mqttReconnectTimer.detach(); // remove the automatic reconnect
  }

  // wifiReconnectTimer.once(2, reconnectWifi);
}

void onWifiConnect(const WiFiEventStationModeGotIP & event) {
  if (settings.settings.mqtt_enabled) {
    if (debug_serial_output) Serial.printf("\nMQTT Connect : Network detected");
    connectToMqtt();
  }
}


void onMqttConnect(bool sessionPresent) {
  if (debug_serial_output) Serial.printf("\nMQTT Connect : Acknowledged. SessionPresent: %s", sessionPresent ? "yes" : "no");
  resubscribeTopics();
  publishLightStates();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    if (debug_serial_output) Serial.printf("\nMQTT Disconnect : reason: %d.  Attempting reconnect as network has not failed.", reason);
    mqttReconnectTimer.once(2, connectToMqtt);
  }
  else {
    if (debug_serial_output) Serial.printf("\nMQTT Disconnect : reason: %d", reason);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  if (debug_serial_output) Serial.printf("\nMQTT Subscribe : Acknowledged packetId: %d, qos: %d", packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  if (debug_serial_output) Serial.printf("\nMQTT Unsubscribe : Acknowledged packetId: %d", packetId);
}

void onMqttPublish(uint16_t packetId) {
  if (debug_serial_output) Serial.printf("\nMQTT Publish : Acknowledged packetId: %d", packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  mqttCallback(topic, payload, len, index, total);
}




