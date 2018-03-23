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

#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>    // https://github.com/me-no-dev/ESPAsyncWebServer
#include <WiFiUdp.h>

#include "StreamString.h"
#include "localwebsite.h"
#include "localwebsite_en.h"

//#include <FastLED.h>
#include "settings.h"

void scheduleReboot();
void scheduleEraseAllSettings();
void turnLightOn(int n);
void turnLightOff(int n);
bool getLightState(int index);


LocalWebsite::LocalWebsite(bool serial_debug)
{
  _serial_output = serial_debug;
  _server = NULL;
  _username = NULL;
  _password = NULL;
  _authenticated = false;
}

void LocalWebsite::setup(AsyncWebServer *server, Settings *settings, const char * username, const char * password) {
  _server = server;
  _settings = settings;
  _username = (char *)username;
  _password = (char *)password;

  _server->on("/", [&](AsyncWebServerRequest * request) {
    handleRoot(request);
  });
  _server->on("/toggle", [&](AsyncWebServerRequest * request) {
    handleToggleLS(request);
  });
  _server->on("/version", HTTP_GET, [&](AsyncWebServerRequest * request) {
    handleVersion(request);
  });
  _server->on("/reboot", [&](AsyncWebServerRequest * request) {
    handleReboot(request);
  });
  _server->on("/erasesettings", [&](AsyncWebServerRequest * request) {
    handleEraseSettings(request);
  });
  _server->on("/settings", HTTP_ANY, [&](AsyncWebServerRequest * request) {
    handleSettings(request);
  });
  _server->on("/showmqtt", HTTP_ANY, [&](AsyncWebServerRequest * request) {
    handleShowMqtt(request);
  });
  _server->on("/lightsettings", [&](AsyncWebServerRequest * request) {
    handleLightSettings(request);
  });
  _server->on("/res", [&](AsyncWebServerRequest * request) {
    handleResource(request);
  });
  _server->on("/stats", [&](AsyncWebServerRequest * request) {
    handleStats(request);
  });

  _server->on("/webupdate", HTTP_GET, [&](AsyncWebServerRequest * request) {
    handleWebUpdate(request);
  });
  _server->on("/update", HTTP_POST,
  [&](AsyncWebServerRequest * request) {
    handleWebUpdateUpload(request);
  },
  [&](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleWebUpdateUploadFile(request, filename, index, data, len, final);
  });
}

/**
   Retrieve a resource.
*/
void LocalWebsite::handleResource(AsyncWebServerRequest *request)
{
  
#ifndef ESTIMATES_MIN_HEAP_MEMORY
#define ESTIMATES_MIN_HEAP_MEMORY 13000
#endif
  
  if (outOfHeapMemory(request, ESTIMATES_MIN_HEAP_MEMORY, true)) {
    request->send_P(404, CONTENT_TYPE_HTML_P, OUT_OF_MEMORY);
    return;
  }

  String fileName = request->arg("file");

  Serial.printf("\nWebsite : filename: %s", fileName.c_str());

  if (fileName != "") {

    if (fileName == "light-off.svg") {
      send_P_cached(request, 200, CONTENT_TYPE_SVG_P, LIGHT_BULB_OFF_SVG);
      return;
    }

    if (fileName == "light-on.svg") {
      send_P_cached(request, 200, CONTENT_TYPE_SVG_P, LIGHT_BULB_ON_SVG);
      return;
    }

    if (fileName == "common.css") {
      send_P_cached(request, 200, CONTENT_TYPE_CSS_P, RES_COMMON_CSS);
      return;
    }

    if (fileName == "common1.js") {
      send_P_cached(request, 200, CONTENT_TYPE_JS_P, RES_COMMON1_JS);
      return;
    }

    if (fileName == "settings.js") {
      send_P_cached(request, 200, CONTENT_TYPE_JS_P, RES_SETTINGS_JS);
      return;
    }
  }

  request->send_P(404, CONTENT_TYPE_HTML_P, FILE_NOT_FOUND_HTML);

}

void LocalWebsite::handleVersion(AsyncWebServerRequest *request)
{
  char f[100];
  snprintf_P(
    f, sizeof(f),
    PSTR("GapLit v%ld.%ld.%ld"),
    (_settings->settings.version & 0xFF000000) >> 24,
    (_settings->settings.version & 0x00FF0000) >> 16,
    (_settings->settings.version & 0x0000FFFF) >> 0);

  request->send(200, CONTENT_TYPE_TEXT, f);
}

void LocalWebsite::handleRoot(AsyncWebServerRequest *request)
{
  if (request->hasArg("refresh")) {
    handleRefreshLS(request);
  }
  else {
    request->send_P(200, CONTENT_TYPE_HTML_P, INDEX_HTML);
  }
}

void LocalWebsite::handleWebUpdate(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, CONTENT_TYPE_HTML, UPDATE_PAGE_HTML);
  response->addHeader("Connection", "close");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void LocalWebsite::handleWebUpdateUpload(AsyncWebServerRequest *request)
{
  if (!Update.hasError())
  {
    request->redirect("/reboot?reboot=msg");
  }

  // the request handler is triggered after the upload has finished...
  // create the response, add header, and send response
  AsyncWebServerResponse *response;
  response = request->beginResponse(200, CONTENT_TYPE_TEXT, (Update.hasError()) ? "FAIL" : "OK");
  response->addHeader("Connection", "close");
  response->addHeader("Access-Control-Allow-Origin", "*");
  scheduleReboot();
  request->send(response);
}

void LocalWebsite::handleWebUpdateUploadFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  if (!index) { // if index == 0 then this is the first frame of data
    Serial.printf("\nWebUpdate: filename: %s", filename.c_str());
    Serial.setDebugOutput(true);

    // calculate sketch space required for the update
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) { //start with max available size
      Update.printError(Serial);
    }
    Update.runAsync(true); // tell the updaterClass to run in async mode
  }

  //Write chunked data to the free sketch space
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) { // if the final flag is set then this is the last frame of data
    if (Update.end(true)) { //true to set the size to the current progress
      Serial.printf("\nWebUpdate: success: %u B\nRebooting...\n", index + len);
    } else {
      Update.printError(Serial);
    }
    Serial.setDebugOutput(false);
  }
}

const char REBOOTING_ALERT_JS[] = "displayMsg('Rebooting.... Please wait 20 seconds');setTimeout(function(){location.href=\"/\"},20000);";

void LocalWebsite::handleReboot(AsyncWebServerRequest *request)
{
  /*
    if(_username != NULL && _password != NULL && !_server->authenticate(_username, _password))
    return _server->requestAuthentication();
  */

  dumpPostArgs(request, "handleReboot");

  String STRING_TRUE = "true";
  if (request->hasArg("reboot") && STRING_TRUE == request->arg("reboot")) {
    scheduleReboot();
    request->send_P(200, CONTENT_TYPE_HTML_P, REBOOTING_ALERT_JS);
  } else {
    request->send_P(200, CONTENT_TYPE_HTML_P, REBOOTING_HTML);
  }
}


void LocalWebsite::handleEraseSettings(AsyncWebServerRequest *request)
{
  if (_username != NULL && _password != NULL && !request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

  dumpPostArgs(request, "handleEraseSettings");

  scheduleEraseAllSettings();

  request->redirect("/reboot?reboot=msg");
}

void LocalWebsite::handleStats(AsyncWebServerRequest *request)
{
  /*
  
  One day it might be necessary to hide stats information.
  
  if (_username != NULL && _password != NULL && !request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }
  */
  uint32_t heapSize = ESP.getFreeHeap();

  String d = "";

  d += "{\n";

  d += "\"heap\" : \""; d += String(heapSize); d += "\",\n";
  d += "\"resetReason\" : \""; d += String(ESP.getResetReason()); d += "\",\n";
  d += "\"chipId\" : \""; d += String(ESP.getChipId()); d += "\",\n";
  d += "\"coreVersion\" : \""; d += String(ESP.getCoreVersion()); d += "\",\n";
  d += "\"sdkVersion\" : \""; d += String(ESP.getSdkVersion()); d += "\",\n";
  d += "\"cpuFreqMHz\" : \""; d += String(ESP.getCpuFreqMHz()); d += "\",\n";
  d += "\"sketchSize\" : \""; d += String(ESP.getSketchSize()); d += "\",\n";
  d += "\"freeSketchSpace\" : \""; d += String(ESP.getFreeSketchSpace()); d += "\",\n";
  d += "\"sketchMD5\" : \""; d += String(ESP.getSketchMD5()); d += "\",\n";
  d += "\"flashChipId\" : \""; d += String(ESP.getFlashChipId()); d += "\",\n";
  d += "\"flashChipSize\" : \""; d += String(ESP.getFlashChipSize()); d += "\",\n";
  d += "\"flashChipRealSize\" : \""; d += String(ESP.getFlashChipRealSize()); d += "\",\n";
  d += "\"flashChipSpeed\" : \""; d += String(ESP.getFlashChipSpeed()); d += "\",\n";
  d += "\"cycleCount\" : \""; d += String(ESP.getCycleCount()); d += "\" \n";
  
  d += "}\n";

  request->send(200, CONTENT_TYPE_HTML, d.c_str());
}

void LocalWebsite::dumpPostArgs(AsyncWebServerRequest *request, const char *methodName)
{
  int args = request->args();

  for (int i = 0; i < args; i++) {
    Serial.printf("\nWebsite: %s called - arg[%s]: %s", methodName, request->argName(i).c_str(), request->arg(i).c_str());
  }

  if (args == 0) {
    Serial.printf("\nWebsite: %s called - no arguments", methodName);
  }
}

void LocalWebsite::handleSettings(AsyncWebServerRequest *request)
{
  String userName = String(_settings->settings.web_user);
  String pwd = String(_settings->settings.web_password);
  
  if (userName.length() > 0 && pwd.length() > 0 && request->authenticate(userName.c_str(), pwd.c_str())) {
    return request->requestAuthentication();
  }

  dumpPostArgs(request, "handleSettings");

  String STRING_TRUE = "true";

  if (request->hasArg("save") && STRING_TRUE == request->arg("save")) {
    handleSettingsSave(request);
  }
  else if (request->hasArg("refresh") && STRING_TRUE == request->arg("refresh")) {
    handleSettingsLoad(request);
  }
  else {
    handleSettingsDisplay(request);
  }
}

void LocalWebsite::handleShowMqtt(AsyncWebServerRequest *request)
{
  if (_username != NULL && _password != NULL && !request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

  dumpPostArgs(request, "handleShowMqtt");

  String STRING_TRUE = "true";

  if (request->hasArg("refresh") && STRING_TRUE == request->arg("refresh")) {
    handleShowMqttRefresh(request);
  }
  else {
    handleShowMqttDisplay(request);
  }
}




void LocalWebsite::handleLightSettings(AsyncWebServerRequest *request)
{
  if (_username != NULL && _password != NULL && !request->authenticate(_username, _password)) {
    return request->requestAuthentication();
  }

  dumpPostArgs(request, "handleLightSettings");

  String STRING_TRUE = "true";

  if (request->hasArg("save") && STRING_TRUE == request->arg("save")) {
    handleSettingsSave(request);
  }
  else if (request->hasArg("refresh") && STRING_TRUE == request->arg("refresh")) {
    handleLightSettingsLoad(request);
  }
  else {
    handleLightSettingsDisplay(request);
  }
}



void LocalWebsite::appendSettingHtml(String &d, int index, const char *label, const char *settingName, int16_t settingValue, const char *fieldType) {
  char v[255];
  snprintf_P(v, sizeof(v), PSTR("updateSetting(%d,'%s', '%s', %d, '%s');"), index, label, settingName, settingValue, fieldType);
  d += v;
}

void LocalWebsite::appendSettingHtml(String &d, int index, const char *label, const char *settingName, const char * settingValue, const char *fieldType) {
  char v[255];
  snprintf_P(v, sizeof(v), PSTR("updateSetting(%d,'%s', '%s', '%s', '%s');"), index, label, settingName, settingValue, fieldType);
  d += v;
}

#define MAX_APPENDSETTINGS_HTML_SWITCH_ITEM     99
#define APPENDSETTINGS_HTML_SWITCH_ITEM( AA , BB , CC, DD )    case AA: appendSettingHtml(d, AA , BB , CC ,  DD ,  "text"); break;
#define APPENDSETTINGS_HTML_SWITCH_ITEM_PASSWORD( AA , BB , CC, DD )    case AA: appendSettingHtml(d, AA , BB , CC , DD , "password" ); break;

// Create the JS String and append to string
void LocalWebsite::appendLightSettingUpdateJS(String &d, int settingId, int index) {
  char v[255];
  snprintf_P(v, sizeof(v), PSTR("updateLightBlock(%d, 'Light %d', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);"),
             settingId, index + 1, _settings->settings.ls_displayName[index], _settings->settings.ls_topicIndex[index],
             _settings->settings.ls_startPixel[index], _settings->settings.ls_endPixel[index],
             _settings->settings.ls_density[index],
             _settings->settings.ls_powerOnState[index],
             _settings->settings.ls_colourOn[index][0], _settings->settings.ls_colourOn[index][1], _settings->settings.ls_colourOn[index][2],
             _settings->settings.ls_colourOff[index][0], _settings->settings.ls_colourOff[index][1], _settings->settings.ls_colourOff[index][2]);
  d += v;
}

#define LIGHT_SEGMENTS_FIRST_SETTING_INDEX  101

void LocalWebsite::appendSettingsHtml(String &d, int index) {

  if (index >= LIGHT_SEGMENTS_FIRST_SETTING_INDEX && index < (LIGHT_SEGMENTS_FIRST_SETTING_INDEX + LIGHT_SEGMENTS_MAX)) {
    appendLightSettingUpdateJS(d, index, index - LIGHT_SEGMENTS_FIRST_SETTING_INDEX);
    return;
  }

  switch (index)
  {
      APPENDSETTINGS_HTML_SWITCH_ITEM (1, "Hostname Template", "hostname", _settings->settings.hostname)
      APPENDSETTINGS_HTML_SWITCH_ITEM (2, "MQTT Host", "mqtt_host", _settings->settings.mqtt_host)
      APPENDSETTINGS_HTML_SWITCH_ITEM (3, "MQTT Port", "mqtt_port", _settings->settings.mqtt_port)
      APPENDSETTINGS_HTML_SWITCH_ITEM (4, "MQTT Client", "mqtt_client", _settings->settings.mqtt_client)
      APPENDSETTINGS_HTML_SWITCH_ITEM (5, "MQTT User", "mqtt_user", _settings->settings.mqtt_user)
      APPENDSETTINGS_HTML_SWITCH_ITEM_PASSWORD (6, "MQTT Password", "mqtt_pwd", _settings->settings.mqtt_pwd)
      APPENDSETTINGS_HTML_SWITCH_ITEM (7, "MQTT Topic", "mqtt_topic", _settings->settings.mqtt_topic)
      APPENDSETTINGS_HTML_SWITCH_ITEM (8, "Button Topic", "button_topic", _settings->settings.button_topic)
      APPENDSETTINGS_HTML_SWITCH_ITEM (9, "MQTT Group Topic", "mqtt_grptopic", _settings->settings.mqtt_grptopic)
      APPENDSETTINGS_HTML_SWITCH_ITEM (10, "MQTT Retry Seconds", "mqtt_retry", _settings->settings.mqtt_retry)
      APPENDSETTINGS_HTML_SWITCH_ITEM (11, "MQTT Enabled", "mqtt_enabled", _settings->settings.mqtt_enabled)
      APPENDSETTINGS_HTML_SWITCH_ITEM (24, "Web Username", "web_user", _settings->settings.web_user)
      APPENDSETTINGS_HTML_SWITCH_ITEM_PASSWORD (25, "Web Password", "web_pwd", _settings->settings.web_password)
      APPENDSETTINGS_HTML_SWITCH_ITEM (26, "Led Strip PIN", "ls_gpio", _settings->settings.ls_gpio)
      APPENDSETTINGS_HTML_SWITCH_ITEM (27, "Status PIN", "status_light_gpio", _settings->settings.status_light_gpio)

      APPENDSETTINGS_HTML_SWITCH_ITEM (28, "Max Strip LEDs", "pixels", _settings->settings.ls_pixels)

      APPENDSETTINGS_HTML_SWITCH_ITEM (30, "Tracer LEDs", "tracerEffectPixels", _settings->settings.ls_tracerPixels)
      APPENDSETTINGS_HTML_SWITCH_ITEM (31, "Tracer Colour Red", "tracerColourR", _settings->settings.ls_tracerColour[0])
      APPENDSETTINGS_HTML_SWITCH_ITEM (32, "Tracer Colour Green", "tracerColourG", _settings->settings.ls_tracerColour[1])
      APPENDSETTINGS_HTML_SWITCH_ITEM (33, "Tracer Colour Blue", "tracerColourB", _settings->settings.ls_tracerColour[2])

      APPENDSETTINGS_HTML_SWITCH_ITEM (40, "Relay PIN", "relay_gpio", _settings->settings.relay_gpio)
      APPENDSETTINGS_HTML_SWITCH_ITEM (41, "Relay Start Delay", "relay_start_delay", _settings->settings.relay_start_delay)
      APPENDSETTINGS_HTML_SWITCH_ITEM (42, "Relay Stop Delay", "relay_stop_delay", _settings->settings.relay_stop_delay)

    default:
      break;
  }

}

void LocalWebsite::handleSettingsLoad(AsyncWebServerRequest *request)
{
  String d = "";

  // Add the general settings
  for (int n = 1; n <= MAX_APPENDSETTINGS_HTML_SWITCH_ITEM; n++)
  {
    appendSettingsHtml(d, n);
  }

  request->send(200, CONTENT_TYPE_HTML, d.c_str());
}

void LocalWebsite::handleLightSettingsLoad(AsyncWebServerRequest *request)
{
  String d = "";

  for (int n = LIGHT_SEGMENTS_FIRST_SETTING_INDEX; n < (LIGHT_SEGMENTS_FIRST_SETTING_INDEX + LIGHT_SEGMENTS_MAX); n++)
  {
    appendSettingsHtml(d, n);
  }

  request->send(200, CONTENT_TYPE_HTML, d.c_str());
}

int LocalWebsite::getHttpIntParam(AsyncWebServerRequest *request, const char *paramName, int minValue, int maxValue, int defaultValue) {

  int paramValue =  request->arg(paramName).toInt();

  if (paramValue >= minValue && paramValue <= maxValue) {
    return paramValue;
  }

  return defaultValue;
}

void LocalWebsite::handleSettingsSave(AsyncWebServerRequest *request) {

  Serial.printf("\nWebsite: handleSettingsSave start - current heap : %u", ESP.getFreeHeap() );

  int n = request->arg("index").toInt();
  String d = "";

  bool processed = false;

  if (n >= LIGHT_SEGMENTS_FIRST_SETTING_INDEX && n < (LIGHT_SEGMENTS_FIRST_SETTING_INDEX + LIGHT_SEGMENTS_MAX)) {

    int m = n - LIGHT_SEGMENTS_FIRST_SETTING_INDEX;

    strlcpy(_settings->settings.ls_displayName[m], request->arg("displayName" ).c_str(), sizeof(_settings->settings.ls_displayName[m]));
    _settings->settings.ls_topicIndex[m] = request->arg("topic" ).toInt();
    _settings->settings.ls_startPixel[m] = request->arg( "startPixel" ).toInt();
    _settings->settings.ls_endPixel[m] = request->arg( "endPixel" ).toInt();
    _settings->settings.ls_density[m] = request->arg( "density" ).toInt();
    _settings->settings.ls_powerOnState[m] = request->arg( "powerOnState" ).toInt();
    _settings->settings.ls_colourOn[m][0] = request->arg( "conR" ).toInt();
    _settings->settings.ls_colourOn[m][1] = request->arg( "conG" ).toInt();
    _settings->settings.ls_colourOn[m][2] = request->arg( "conB" ).toInt();
    _settings->settings.ls_colourOff[m][0] = request->arg( "coffR" ).toInt();
    _settings->settings.ls_colourOff[m][1] = request->arg( "coffG" ).toInt();
    _settings->settings.ls_colourOff[m][2] = request->arg( "coffB" ).toInt();

    processed = true;
  }

  int i;

  if (!processed)
  {

    switch (n)
    {
      case 1:
        strlcpy(_settings->settings.hostname, request->arg("value").c_str(), sizeof(_settings->settings.hostname));
        break;
      case 2:
        strlcpy(_settings->settings.mqtt_host, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_host));
        break;
      case 3:
        _settings->settings.mqtt_port = getHttpIntParam(request, "value", 1, 32000, _settings->settings.mqtt_port);
        break;
      case 4:
        strlcpy(_settings->settings.mqtt_client, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_client));
        // composeSetting(mqtt_client, _settings->settings.mqtt_client, sizeof(mqtt_client));
        break;
      case 5:
        strlcpy(_settings->settings.mqtt_user, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_user));
        break;
      case 6:
        strlcpy(_settings->settings.mqtt_pwd, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_pwd));
        break;
      case 7:
        strlcpy(_settings->settings.mqtt_topic, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_topic));
        // composeSetting(mqtt_topic, _settings->settings.mqtt_topic, sizeof(mqtt_topic));
        break;
      case 8:
        strlcpy(_settings->settings.button_topic, request->arg("value").c_str(), sizeof(_settings->settings.button_topic));
        break;
      case 9:
        strlcpy(_settings->settings.mqtt_grptopic, request->arg("value").c_str(), sizeof(_settings->settings.mqtt_grptopic));
        break;
      case 10:
        _settings->settings.mqtt_retry = getHttpIntParam(request, "value", 0, 3600, _settings->settings.mqtt_retry);
        break;
      case 11:
        _settings->settings.mqtt_enabled = getHttpIntParam(request, "value", 0, 1, _settings->settings.mqtt_enabled);
        break;
      case 24:
        strlcpy(_settings->settings.web_user, request->arg("value").c_str(), sizeof(_settings->settings.web_user));
        break;
      case 25:
        strlcpy(_settings->settings.web_password, request->arg("value").c_str(), sizeof(_settings->settings.web_password));
        break;
      case 26:
        _settings->settings.ls_gpio = (byte) getHttpIntParam(request, "value", 0, 255, _settings->settings.ls_gpio);
        break;
      case 27:
        _settings->settings.status_light_gpio = (byte) getHttpIntParam(request, "value", 0, 255, _settings->settings.status_light_gpio);
        break;

      case 28:
        _settings->settings.ls_pixels = getHttpIntParam(request, "value", 0, 500, _settings->settings.ls_pixels);
        break;
        

      case 30:
        _settings->settings.ls_tracerPixels = getHttpIntParam(request, "value", 0, 255, _settings->settings.ls_tracerPixels);
        break;

      case 31:
        _settings->settings.ls_tracerColour[0] = getHttpIntParam(request, "value", 0, 255, _settings->settings.ls_tracerColour[0]);
        break;
      case 32:
        _settings->settings.ls_tracerColour[1] = getHttpIntParam(request, "value", 0, 255, _settings->settings.ls_tracerColour[1]);
        break;
      case 33:
        _settings->settings.ls_tracerColour[2] = getHttpIntParam(request, "value", 0, 255, _settings->settings.ls_tracerColour[2]);
        break;

      case 40:
        _settings->settings.relay_gpio = getHttpIntParam(request, "value", -1, 20, _settings->settings.relay_gpio);
        break;
      case 41:
        _settings->settings.relay_start_delay = getHttpIntParam(request, "value", 0, 31000, _settings->settings.relay_start_delay);
        break;
      case 42:
        _settings->settings.relay_stop_delay = getHttpIntParam(request, "value", 0, 31000, _settings->settings.relay_stop_delay);
        break;


      case 0:
        // Special control case - store all the settings in non-volatile
        {
          _settings->saveAll();
          d += "displayMsg('Settings updated');";
        }
        break;
      default:
        returnFail(request, "Unable to save the item");
        return;
        break;
    }
  }

  appendSettingsHtml(d, n);

  Serial.printf("\nWebsite: handleSettingsSave end  - current heap : %u", ESP.getFreeHeap() );

  request->send(200, CONTENT_TYPE_HTML, d.c_str());

}


void LocalWebsite::handleShowMqttRefresh(AsyncWebServerRequest *request)
{
  String d = "";
  char smallValue[33];
  char mqtt_topic[33];

  Serial.printf("\nWebsite: handleShowMqttRefresh called - current heap : %u", ESP.getFreeHeap() );

  _settings->getHostname(smallValue, sizeof(smallValue));
  d += "showSNV('Global', 'Eval Hostname', '" +  String(smallValue) + "');";

  _settings->composeSetting(smallValue, _settings->settings.mqtt_client, sizeof(smallValue));
  d += "showSNV('Global', 'Eval mqtt_client', '" +  String(smallValue) + "');";

  _settings->composeSetting(mqtt_topic, _settings->settings.mqtt_topic, sizeof(mqtt_topic));
  d += "showSNV('Global', 'Eval mqtt_topic', '" +  String(mqtt_topic) + "');";

  d += "showSNV('Global', 'MQTT Enabled', '" +  String(_settings->settings.mqtt_enabled ? "True" : "False") + "');";
  d += "showSNV('Global', 'MQTT State off', '" +  String(_settings->settings.mqtt_state_text[0]) + "');";
  d += "showSNV('Global', 'MQTT State on', '" +  String(_settings->settings.mqtt_state_text[1]) + "');";
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    char topic[120];
    String suffix = String(MQTT_SUFFIX);
    suffix += String(_settings->settings.ls_topicIndex[n]);
    _settings->composeMqttTopic(topic, sizeof(topic), 1, mqtt_topic, suffix.c_str());
    d += "showSNV('States', 'Light " + String(n + 1) + "', '" +  String(topic) + "');";
  }
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    char topic[120];
    String suffix = String(MQTT_SUFFIX);
    suffix += String(_settings->settings.ls_topicIndex[n]);
    _settings->composeMqttTopic(topic, sizeof(topic), 0, mqtt_topic, suffix.c_str());
    d += "showSNV('Commands', 'Light " + String(n + 1) + "', '" +  String(topic) + "');";
  }
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    d += "showSNV('Topic IDs', 'Light " + String(n + 1) + "', '" +  (_settings->settings.ls_topicIndex[n] > 0 ? String(_settings->settings.ls_topicIndex[n]) : String("disabled")) + "');";
  }

  Serial.printf("\nWebsite: handleShowMqttRefresh end  - current heap : %u", ESP.getFreeHeap() );

  request->send(200, CONTENT_TYPE_HTML, d.c_str());
}




void LocalWebsite::handleSettingsDisplay(AsyncWebServerRequest *request) {
  request->send_P(200, CONTENT_TYPE_HTML_P, SETTINGS_HTML);
}

void LocalWebsite::handleShowMqttDisplay(AsyncWebServerRequest *request) {
  request->send_P(200, CONTENT_TYPE_HTML_P, SHOWMQTT_HTML);
}


void LocalWebsite::handleLightSettingsDisplay(AsyncWebServerRequest *request) {
  request->send_P(200, CONTENT_TYPE_HTML_P, LIGHT_SETTINGS_HTML);
}

void LocalWebsite::returnFail(AsyncWebServerRequest *request, String msg)
{
  AsyncWebServerResponse *response = request->beginResponse(500, CONTENT_TYPE_TEXT, msg + "\r\n");
  response->addHeader("Connection", "close");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void LocalWebsite::handleToggleLS(AsyncWebServerRequest *request) {

  String s;
  int lightIndex = 0;

  if (!request->hasArg("light")) return returnFail(request, "BAD ARGS");

  s = request->arg("light");
  lightIndex = s.toInt() - 1;
  if (lightIndex < 0 || lightIndex >= LIGHT_SEGMENTS_MAX) {
    return;
  }

  Serial.printf("\nToggleLS : lightIndex=%d", lightIndex);

  if (!getLightState(lightIndex)) {
    turnLightOn(lightIndex);
  }
  else {
    turnLightOff(lightIndex);
  }

  returnLightSwitchStatus(request);
}


void LocalWebsite::handleRefreshLS(AsyncWebServerRequest *request)
{
  if (!request->hasArg("refresh")) return returnFail(request, "BAD ARGS");
  String refreshValue = request->arg("refresh");
  returnLightSwitchStatus(request);
}

void LocalWebsite::returnLightSwitchStatus(AsyncWebServerRequest *request)
{
  const char *lightState = "updateLS({num}, '{label}', '{state}');";
  String htmlLightStates = "";
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {

    // If the topic is 0 then it is disabled
    if (_settings->settings.ls_topicIndex[n] == 0)
      continue;

    String s = lightState;
    s.replace(F("{num}"), String(n + 1));
    s.replace(F("{label}"), String(_settings->settings.ls_displayName[n]));
    s.replace(F("{state}"), getLightState(n) ? F("On") : F("Off"));
    htmlLightStates += s;
  }

  request->send(200, CONTENT_TYPE_HTML, htmlLightStates.c_str());
}


void LocalWebsite::returnOK(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, CONTENT_TYPE_TEXT, "OK\r\n");
  response->addHeader("Server", "ESP Async Web Server");
  request->send(response);
}

void LocalWebsite::handleNotFound(AsyncWebServerRequest *request)
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, CONTENT_TYPE_TEXT, message);
}


/*
  Check if there is an appropriate amount of heap space left to process a request
*/
bool LocalWebsite::outOfHeapMemory(AsyncWebServerRequest *request, long reservedMemory, bool waitForMemory) {

  if (ESP.getFreeHeap() > reservedMemory) {
    return false;
  }

  Serial.printf("\nWebsite : insufficient heap memory - current heap : %u", ESP.getFreeHeap() );

  return true;
}

/*
   Respond with a cachable PROGMEM resource file
*/
void LocalWebsite::send_P_cached(AsyncWebServerRequest *request, int code, const String& contentType, PGM_P content)
{
  AsyncWebServerResponse *response = request->beginResponse_P(code, contentType, content, nullptr);
#ifndef DO_NOT_SEND_HTTP_CACHE_HEADERS
  response->addHeader("Cache-Control", "private, max-age=3600");
  String etag = String(sizeof(content));
  response->addHeader("ETag", etag);
#endif
  request->send(response);
}

