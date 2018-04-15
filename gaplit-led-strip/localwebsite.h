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
#ifndef __LocalWebsite_H_
#define __LocalWebsite_H_

class AsyncWebServer;
class AsyncWebServerRequest;
class Settings;

class LocalWebsite
{
  public:
    LocalWebsite(bool serial_debug = false);

    void setup(AsyncWebServer *server, Settings *settings)
    {
      setup(server, settings, NULL, NULL);
    }

    void setup(AsyncWebServer *server, Settings *settings, const char * username, const char * password);

  protected:
    void handleRoot(AsyncWebServerRequest *request);
    void handleReboot(AsyncWebServerRequest *request);
    void handleLightSettings(AsyncWebServerRequest *request);
    void appendSettingHtml(String &d, int index, const char *label, const char *settingName, int16_t settingValue, const char *fieldType);
    void appendSettingHtml(String &d, int index, const char *label, const char *settingName, const char * settingValue, const char *fieldType);
    void appendLightSettingUpdateJS(String &d, int settingId, int index);
    void appendSettingsHtml(String &d, int index);
    void dumpPostArgs(AsyncWebServerRequest *request, const char *methodName);
    void handleStats(AsyncWebServerRequest *request);
    void handleSettings(AsyncWebServerRequest *request);
    void handleShowMqtt(AsyncWebServerRequest *request);
    void handleSettingsLoad(AsyncWebServerRequest *request);
    void handleLightSettingsLoad(AsyncWebServerRequest *request);
    int getHttpIntOfParamValue();
    void handleSettingsSave(AsyncWebServerRequest *request);
    void handleSettingsDisplay(AsyncWebServerRequest *request);
    void handleShowMqttDisplay(AsyncWebServerRequest *request);
    void handleShowMqttRefresh(AsyncWebServerRequest *request);
    void handleLightSettingsDisplay(AsyncWebServerRequest *request);
    void returnFail(AsyncWebServerRequest *request, String msg);
    void handleToggleLS(AsyncWebServerRequest *request);
    void handleRefreshLS(AsyncWebServerRequest *request);
    void returnLightSwitchStatus(AsyncWebServerRequest *request);
    void returnOK(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    void handleWebUpdate(AsyncWebServerRequest *request);
    void handleWebUpdateUpload(AsyncWebServerRequest *request);
    void handleWebUpdateUploadFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    int getHttpIntParam(AsyncWebServerRequest *request, const char *paramName, int minValue, int maxValue, int defaultValue);
    void handleResource(AsyncWebServerRequest *request);
    void handleVersion(AsyncWebServerRequest *request);
    void handleEraseSettings(AsyncWebServerRequest *request);
    bool outOfHeapMemory(AsyncWebServerRequest *request, long reservedMemory, bool waitForMemory);
    void send_P_cached(AsyncWebServerRequest *request, int code, const String& contentType, PGM_P content);

  private:
    bool _serial_output;
    AsyncWebServer *_server;
    Settings *_settings;
    char * _username;
    char * _password;
    bool _authenticated;
    String _updaterError;
};

#endif  // __LocalWebsite_H_

