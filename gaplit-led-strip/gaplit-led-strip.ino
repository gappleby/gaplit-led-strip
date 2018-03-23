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

#include "settings.h"
#include "localwebsite.h"

char host_name[33];
char mqtt_client[33];
char mqtt_topic[33];

#define REBOOT_DISABLED -1
int reboot_counter = REBOOT_DISABLED;

#define ERASE_FLASH_DISABLED -1
int erase_flash_counter = ERASE_FLASH_DISABLED;

// Relay On/Off states and registers
#define RELAY_OFF 0
#define RELAY_ON  1
int relay_state_current = RELAY_OFF;
int relay_state_next = RELAY_OFF;
uint32_t relay_change_after = 0;

// NeoPixel setup
#define MAX_NUM_LEDS 500
typedef struct _CRGB {
  byte r;  byte b;  byte g;
} CRGB;
CRGB leds[MAX_NUM_LEDS];
CRGB targetLeds[MAX_NUM_LEDS];
Adafruit_NeoPixel stripLeds = Adafruit_NeoPixel();

//LightConfiguration lights[LIGHT_SEGMENTS_MAX];
bool currentState[LIGHT_SEGMENTS_MAX];

const long STATUS_RESEND_PERIOD =  60000L;
long lastStatusSent = millis();

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

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
// Ticker wifiReconnectTimer;

Settings settings;

void setup() {
  // Start the serial line for debugging
  Serial.begin(115200);

  // Load the settings
  settings.load();
  settings.loadDelta();

  // Don't bother to dump the settings normally. 
  // Really just used to diagnose upgrade issues between 
  // version where the config values have changed.
  //settings.dumpSettings();

  blinkLed(1, 100);

  // Configure settings
  settings.composeSetting(mqtt_client, settings.settings.mqtt_client, sizeof(mqtt_client));
  settings.composeSetting(mqtt_topic, settings.settings.mqtt_topic, sizeof(mqtt_topic));
  settings.getHostname(host_name, sizeof(host_name));

  // Setup PINs
  if (settings.settings.status_light_gpio >= 0) pinMode(settings.settings.status_light_gpio, OUTPUT);
  if (settings.settings.relay_gpio >= 0) pinMode(settings.settings.relay_gpio, OUTPUT);

  // stripLeds = Adafruit_NeoPixel(settings.settings.ls_pixels, settings.settings.ls_gpio, NEO_GRB + NEO_KHZ800);
  stripLeds.updateType(NEO_GRB + NEO_KHZ800);
  stripLeds.updateLength(settings.settings.ls_pixels);
  stripLeds.setPin(settings.settings.ls_gpio);
  stripLeds.begin();
  stripLeds.show();

  // Set the wifi host name.  Unless overridden composed using MAC address
  WiFi.hostname(host_name);

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

  initialiseLedBuffers();

  // Publish initial state
  lastStatusSent -= (STATUS_RESEND_PERIOD - 15000L);


}

void loop() {

  reconnectWifi();

  processBackgroundFlags();

  publishLightStates();

  transitionLeds();

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

void initialiseLedBuffers() {
  // Initialise the LEDs.  This is required to ensure that any transitions will occur.
  for (int l = 0; l < settings.settings.ls_pixels; l++)
  {
    leds[l].r = 1; leds[l].g = 1; leds[l].b = 1;
    targetLeds[l].r = 0; targetLeds[l].g = 0; targetLeds[l].b = 0;
  }

  // Load configuration
  for (int n = 0; n < LIGHT_SEGMENTS_MAX ; n++)
  {
    currentState[n] = false;

    if (settings.settings.ls_topicIndex[n] == 0) {
      continue;
    }

    // Turn all the lights on
    if (settings.settings.ls_powerOnState[n] > 0) {
      turnLightOn(n);
    }
  }
}


/**
   LED soft on/off logic (and tracer effect)
*/
long transitionTimer = millis();
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
  if (millis() < transitionTimer)
    return;

  bool showLeds = false;

  for (int n = 0; n < settings.settings.ls_pixels; n++)
  {
    if (leds[n].r != targetLeds[n].r)  {
      leds[n].r = blendValue(leds[n].r, targetLeds[n].r);
      showLeds = true;
    }

    if (leds[n].g != targetLeds[n].g)  {
      leds[n].g = blendValue(leds[n].g, targetLeds[n].g);
      showLeds = true;
    }

    if (leds[n].b != targetLeds[n].b)  {
      leds[n].b = blendValue(leds[n].b, targetLeds[n].b);
      showLeds = true;
    }
  }

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


  if (showLeds)  {
    for (int n = 0; n < settings.settings.ls_pixels; n++) {
      stripLeds.setPixelColor(n, stripLeds.Color(leds[n].r, leds[n].g, leds[n].b));
    }
    stripLeds.show();
    //FastLED.show();
  }

  transitionTimer = millis() + TRANSITION_DELAY;

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

  // Update the relays
  updateRelayState();

}

/**
   MQTT Status Logic
*/

//
// Get the current light state
bool getLightState(int index) {
  return currentState[index];
}


//
// Publish Light State
void publishLightState(int index)
{
  // If enabled
  if (settings.settings.ls_topicIndex[index] > 0) {

    // Publish MQTT
    if (settings.settings.mqtt_enabled) {
      char topic[120];
      String suffix = String(MQTT_SUFFIX);
      suffix += String(settings.settings.ls_topicIndex[index]);
      settings.composeMqttTopic(topic, sizeof(topic), 1, mqtt_topic, suffix.c_str());

      uint16_t packetId = mqttClient.publish(topic, 1, true, currentState[index] ? settings.settings.mqtt_state_text[1] : settings.settings.mqtt_state_text[0]);

      Serial.printf("\nMQTT Publish : Topic=%s , State=%s packetId=%d", topic, currentState[index] ? settings.settings.mqtt_state_text[1] : settings.settings.mqtt_state_text[0], packetId);
    }

  }
}

void publishLightStates()
{
  // Publish every minute, regardless of a change.
  long now = millis();
  if ((now - lastStatusSent > STATUS_RESEND_PERIOD) || (now - lastStatusSent < 0) ) {
  
    lastStatusSent = now;
    
    for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
      publishLightState(n);
    }
    
    if (settings.settings.mqtt_enabled) {
      Serial.printf("\nMQTT Publish : All topics resent");
    }
  
  }
}

//
// Blink the led the number multiple times.
//
void blinkLed(int blinks, int period)
{
  while (blinks-- > 0)
  {
    if (settings.settings.status_light_gpio >= 0) {
      digitalWrite(settings.settings.status_light_gpio, LOW);
    }
    delay(300);
    
    if (settings.settings.status_light_gpio >= 0)  { 
      digitalWrite(settings.settings.status_light_gpio, HIGH);
    }
    delay(period);
  }
}


/**
   Relay Logic control
*/
// 
// If off to on, power on relay and wait for period, then turn on all the lights
// If on to off, turn off the lights and wait the period, then turn off the relay
// If on to on, do nothing
// If off to off, do nothing
// If relay disabled - do nothing - as turn on has done all the work
void updateRelayState()
{
  if (relay_state_current == relay_state_next) {
    return;  // nothing to do
  }
  
  // If no relay then do nothing
  if (settings.settings.relay_gpio < 0) {
    return;
  }
  
  uint32_t now = millis();
  
  if (relay_state_next == RELAY_ON) {
    if (relay_change_after == 0) {
      // Turn on the relay and wait before re-activating the lights
      digitalWrite(settings.settings.relay_gpio, HIGH);
      Serial.printf("\nRelay : ON, wait %d ms", settings.settings.relay_start_delay);

      relay_change_after = now + settings.settings.relay_start_delay;
      if (relay_change_after < now) {
        // It's overflowed - help
        relay_change_after = now; // Just schedule it for the next round
      }
    } else if (relay_change_after < now) {
      // Delay expired
      relay_change_after = 0; // Disable logic - it's all complete
      relay_state_current = RELAY_ON;
      Serial.printf("\nRelay : Turn on the lights after the delay");
      // ready to turn on the lights for real
      for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
        if (currentState[n]) {
          turnLightOn(n);
        }
        else { 
          turnLightOff(n);
        }
      }
    }
  } else if (relay_state_next == RELAY_OFF) {
    if (relay_change_after == 0) {
      
      // ready to turn off the lights for real
      Serial.printf("\nRelay : Turn off the lights, wait %d ms", settings.settings.relay_stop_delay);
      for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
        if (currentState[n]) {
          turnLightOn(n);
        }
        else { 
          turnLightOff(n);
        }
      }

      relay_change_after = now + settings.settings.relay_stop_delay;
      if (relay_change_after < now) {
        // It's overflowed - help
        relay_change_after = now; // Just schedule it for the next round
      }

    } else if (relay_change_after < now) {
      // Delay expired
      relay_change_after = 0; // Disable logic - it's all complete
      relay_state_current = RELAY_OFF;
      // Turn on the relay
      digitalWrite(settings.settings.relay_gpio, LOW);
      Serial.printf("\nRelay : OFF, after the delay");
    }
    
  }
}

void setRelayOn() {

  // If any of the lights need to be on or off, then just set the current state
  if (settings.settings.relay_gpio < 0) {
    relay_state_current = RELAY_ON;
    relay_state_next = RELAY_ON;
    return;
  }

  bool isOn = false;
  
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
      if (currentState[n]) { 
        isOn = true; 
      }
  }

  int target_relay_state_next = isOn ? RELAY_ON : RELAY_OFF;

  if (target_relay_state_next != relay_state_next) {
    if (target_relay_state_next == relay_state_current && relay_state_current == RELAY_ON) {
      relay_change_after = 0;
    }
    relay_state_next = target_relay_state_next;
  }
}


/**
   Virtual Light Segment control
*/
void turnLightOn(int n)
{
  currentState[n] = true;
  publishLightState(n);

  setRelayOn();

  // If relay not on, then just return as it will happen after a short delay
  if (relay_state_current == RELAY_OFF) {
    return;
  }

  if (settings.settings.ls_density[n] <= 0) settings.settings.ls_density[n] = 1;
  
  for (int m = settings.settings.ls_startPixel[n]; m <= settings.settings.ls_endPixel[n]; m += settings.settings.ls_density[n]) {
    CRGB cOn;  cOn.r = settings.settings.ls_colourOn[n][0]; cOn.g = settings.settings.ls_colourOn[n][1]; cOn.b = settings.settings.ls_colourOn[n][2];
    CRGB cOff; cOff.r = settings.settings.ls_colourOff[n][0]; cOff.g = settings.settings.ls_colourOff[n][1]; cOff.b = settings.settings.ls_colourOff[n][2];
    targetLeds[m] = cOn;
  }
}

void turnLightOff(int n)
{
  currentState[n] = false;
  publishLightState(n);

  setRelayOn();

  // If relay not on, then just return as it will happen after a short delay
  if (relay_state_current == RELAY_OFF) {
    return;
  }

  if (settings.settings.ls_density[n] <= 0) settings.settings.ls_density[n] = 1;
  
  for (int m = settings.settings.ls_startPixel[n]; m <= settings.settings.ls_endPixel[n]; m += settings.settings.ls_density[n]) {
    CRGB cOff; cOff.r = settings.settings.ls_colourOff[n][0]; cOff.g = settings.settings.ls_colourOff[n][1]; cOff.b = settings.settings.ls_colourOff[n][2];
    targetLeds[m] = cOff;
  }

}


/**
   MQTT Logic
*/

// MQTT callback function -(Use only if topics are being subscribed to)
void mqttCallback(char* topic, char* payload, size_t length, size_t index, size_t total) {

  Serial.printf("\nMQTT Command : Topic = %s, Payload length = %d", topic, length);

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
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++)
  {
    char targetTopic[120];
    String suffix = String(MQTT_SUFFIX);
    suffix += String(settings.settings.ls_topicIndex[n]);
    settings.composeMqttTopic(targetTopic, sizeof(targetTopic), 0, mqtt_topic, suffix.c_str());

    if (strTopic == targetTopic) {

      updatedLeds += 1;

      String strValue = String((char*)data);

      Serial.printf("\nMQTT Command : Topic = %s, Value = %s", strTopic.c_str(), strValue.c_str());

      if (strValue == settings.settings.mqtt_state_text[1]) {
        turnLightOn(n);
      }

      if (strValue == settings.settings.mqtt_state_text[0]) {
        turnLightOff(n);
      }

      publishLightState(n);
    }
  }

}

void resubscribeTopics() {

  if (!settings.settings.mqtt_enabled) {
    return;
  }

  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++)  {

    if (settings.settings.ls_topicIndex[n] > 0) {

      char targetTopic[120];
      String suffix = String(MQTT_SUFFIX);
      suffix += String(settings.settings.ls_topicIndex[n]);
      settings.composeMqttTopic(targetTopic, sizeof(targetTopic), 0, mqtt_topic, suffix.c_str());

      uint16_t packetIdSub = mqttClient.subscribe(targetTopic, 2);
      Serial.printf("\nMQTT Subscribe : Topic = %s packetId = %d", targetTopic, packetIdSub);

    }
  }
}


/**
   Wifi Client Logic
*/

void connectToMqtt() {

  if (settings.settings.mqtt_enabled) {
    Serial.printf("\nMQTT Connect : Attempting\n");
    mqttClient.connect();
  }

}

void reconnectWifi() {

  if ( WiFi.status() == WL_CONNECTED ) {
    return;
  }

  Serial.printf("\nWifi : Not connected - trying autoConnect with hostname %s\n", host_name);
  wifiManager.autoConnect(host_name);

  // Connect to MQTT
  if ( WiFi.status() != WL_CONNECTED ) {

    Serial.printf("\nWifi : Failed to connect Wifi, try again in 5 seconds");

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

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  
  if (settings.settings.mqtt_enabled) {
    Serial.printf("\nMQTT Disconnected : Lost connection");
    mqttReconnectTimer.detach(); // remove the automatic reconnect
  }

  // wifiReconnectTimer.once(2, reconnectWifi);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
    if (settings.settings.mqtt_enabled) {
      Serial.printf("\nMQTT Connect : Network detected");
      connectToMqtt();
    }
}


void onMqttConnect(bool sessionPresent) {
  Serial.printf("\nMQTT Connect : Acknowledged. SessionPresent: %s", sessionPresent ? "yes" : "no");
  resubscribeTopics();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    Serial.printf("\nMQTT Disconnect : reason: %d.  Attempting reconnect as network has not failed.", reason);
    mqttReconnectTimer.once(2, connectToMqtt);
  }
  else {
    Serial.printf("\nMQTT Disconnect : reason: %d", reason);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf("\nMQTT Subscribe : Acknowledged packetId: %d, qos: %d", packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.printf("\nMQTT Unsubscribe : Acknowledged packetId: %d", packetId);
}

void onMqttPublish(uint16_t packetId) {
  Serial.printf("\nMQTT Publish : Acknowledged packetId: %d", packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  mqttCallback(topic, payload, len, index, total);
}




