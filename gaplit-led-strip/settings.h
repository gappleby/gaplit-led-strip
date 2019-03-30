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
#ifndef _SETTINGS_H_
#define _SETTINGS_H_


// Choose an appropriate default board
#define DEFAULT_BOARD_WEMOS_BOARD
//#define DEFAULT_BOARD_SONOFF_BOARD

#ifdef DEFAULT_BOARD_WEMOS_BOARD
// D1 = 5
// D4 = 2
// D2 = 4
#define LS_GPIO_PIN_DEFAULT     5
#define STATUS_GPIO_PIN_DEFAULT 2
#define RELAY_GPIO_PIN_DEFAULT  -1
#endif

#ifdef DEFAULT_BOARD_SONOFF_BOARD
#define LS_GPIO_PIN_DEFAULT     14
#define STATUS_GPIO_PIN_DEFAULT 13
#define RELAY_GPIO_PIN_DEFAULT  12
#endif


#define VERSION                 0x03000004
#define LIGHT_SEGMENTS_MAX      8
#define MAX_PIXELS_DEFAULT      100
#define DISPLAY_NAME_MAX_LEN    33
#define DISPLAY_NAME_DEFAULT    "LS Light "
#define WEB_USERNAME            "admin"           // Web server Admin mode user name
#define WEB_PASSWORD            ""                // Default password (empty is disabled)


#define PROJECT                "gaplit"          // PROJECT is used as the default topic delimiter and OTA file name

#define MQTT_RETRY_SECS        15           // Minimum seconds to retry MQTT connection
#define MQTT_ENABLED           0

#define WIFI_HOSTNAME          "%s-%04d"    // Expands to <MQTT_TOPIC>-<last 4 decimal chars of MAC address>

#define APP_TIMEZONE           10           // Hours from GMT
#define INPUT_BUFFER_SIZE      255          // Max number of characters in (serial) command buffer
#define OTA_URL                ""
#define SERIAL_LOG_LEVEL       0

#define D_LOG_APPLICATION "APP: "  // Application
#define D_LOG_CONFIG "CFG: "       // Settings
#define D_SAVED_TO_FLASH_AT "Saved to flash at"
#define D_LOADED_FROM_FLASH_AT "Loaded from flash at"
#define D_USE_DEFAULTS "Use defaults"
#define D_ERASED_SECTOR "Erased sector"
#define D_ERASE "Erase"
#define D_RESTARTING "Restarting"
#define D_UNIT_SECTORS "sectors"
#define D_OK "Ok"
#define D_ON "On"
#define D_ERROR "Error"
#define D_COUNT "Count"
#define D_BYTES "Bytes"

#define CFG_HOLDER             0x20180401        // [Reset 1] Change this value to load following default configuration parameters
#define SAVE_DATA              1                 // [SaveData] Save changed parameters to Flash (0 = disable, 1 - 3600 seconds)
#define SAVE_STATE             1                 // [SetOption0] Save changed power state to Flash (0 = disable, 1 = enable)

#define WS2812_LEDS            30                // [Pixels] Number of WS2812 LEDs to start with

#define RELAY_START_DELAY     300              // duration in MS
#define RELAY_STOP_DELAY      10000             // duration in MS

// -- MQTT ----------------------------------------
#define MQTT_TOPIC           PROJECT           // [Topic] (unique) MQTT device topic
#define TELE_PERIOD          300               // [TelePeriod] Telemetry (0 = disable, 10 - 3600 seconds)
#define MQTT_GRPTOPIC        "sonoffs"         // [GroupTopic] MQTT Group topic
#define MQTT_HOST            "hass.local"      // [MqttHost]
#define MQTT_PORT            1883              // [MqttPort] MQTT port (10123 on CloudMQTT)
#define MQTT_USER            "DVES_USER"       // [MqttUser] Optional user
#define MQTT_PASS            "DVES_PASS"       // [MqttPassword] Optional password
#define MQTT_TOKEN_PREFIX      "%prefix%"   // To be substituted by mqtt_prefix[x]
#define MQTT_TOKEN_TOPIC       "%topic%"    // To be substituted by mqtt_topic, mqtt_grptopic, mqtt_buttontopic, mqtt_switchtopic
#define MQTT_FULLTOPIC         "%prefix%/%topic%/" // [FullTopic] Subscribe and Publish full topic name - Legacy topic
#define MQTT_CLIENT_ID       "DVES_%06X"       // [MqttClient] Also fall back topic using Chip Id = last 6 characters of MAC address
#define MQTT_STATUS_OFF        "OFF"             // [StateText1] Command or Status result when turned off (needs to be a string like "0" or "Off")
#define MQTT_STATUS_ON         "ON"              // [StateText2] Command or Status result when turned on (needs to be a string like "1" or "On")
#define MQTT_CMND_TOGGLE       "TOGGLE"          // [StateText3] Command to send when toggling (needs to be a string like "2" or "Toggle")
#define MQTT_CMND_HOLD         "HOLD"            // [StateText4] Command to send when button is kept down for over KEY_HOLD_TIME * 0.1 seconds (needs to be a string like "HOLD")
#define MQTT_SUFFIX             "power"         // Suffix for the status and command

// -- HTTP ----------------------------------------
#define WEB_PORT             80                // Web server Port for User and Admin mode

// %prefix% token options
#define SUB_PREFIX           "cmnd"            // [Prefix1] Sonoff devices subscribe to %prefix%/%topic% being SUB_PREFIX/MQTT_TOPIC and SUB_PREFIX/MQTT_GRPTOPIC
#define PUB_PREFIX           "stat"            // [Prefix2] Sonoff devices publish to %prefix%/%topic% being PUB_PREFIX/MQTT_TOPIC
#define PUB_PREFIX2          "tele"            // [Prefix3] Sonoff devices publish telemetry data to %prefix%/%topic% being PUB_PREFIX2/MQTT_TOPIC/UPTIME, POWER and TIME

const char MQTT_PREFIX[3][5] PROGMEM = {
  SUB_PREFIX,
  PUB_PREFIX,
  PUB_PREFIX2
};

#ifdef __cplusplus
extern "C" {
#endif

struct PersistentSettings {
  unsigned long cfg_holder;
  unsigned long save_flag;
  unsigned long version;
  unsigned long bootcount;

  int16_t       save_data;
  int8_t        timezone;
  char          ota_url[101];
  char          hostname[33];
  char          web_user[33];
  char          web_password[65];
  int8_t        status_light_gpio;
  byte          seriallog_level;

  uint8_t       mqtt_enabled;
  char          mqtt_host[33];
  uint16_t      mqtt_port;
  char          mqtt_client[33];
  char          mqtt_user[33];
  char          mqtt_pwd[33];
  char          mqtt_prefix[3][11];
  char          mqtt_topic[33];
  char          mqtt_fulltopic[100];
  char          button_topic[33];
  char          mqtt_grptopic[33];
  uint16_t      mqtt_retry;
  char          mqtt_switch_topic[33];
  char          mqtt_state_text[4][11];

  int8_t        relay_gpio;           // gpio of the relay (if -1 is disabled)
  uint16_t      relay_start_delay;    // start delay in MS after relay on
  uint16_t      relay_stop_delay;     // stop delay in MS before relay off

  // Tracer Pixels
  uint8_t       ls_tracerPixels;
  uint8_t       ls_tracerColour[3];

  // Spare bytes
  uint8_t       spare[160];           // Reserved for future use.  Use sparingly.

  // Light Strip Settings
  int8_t        ls_gpio;             // gpio of the strip light (if -1 is disabled)
  uint16_t      ls_pixels;
  char          ls_displayName[LIGHT_SEGMENTS_MAX][DISPLAY_NAME_MAX_LEN];
  uint8_t       ls_powerOnState[LIGHT_SEGMENTS_MAX];
  uint8_t       ls_topicIndex[LIGHT_SEGMENTS_MAX];
  uint16_t      ls_startPixel[LIGHT_SEGMENTS_MAX];
  uint16_t      ls_endPixel[LIGHT_SEGMENTS_MAX];
  uint16_t      ls_density[LIGHT_SEGMENTS_MAX];
  uint8_t       ls_colourOn[LIGHT_SEGMENTS_MAX][3];
  uint8_t       ls_colourOff[LIGHT_SEGMENTS_MAX][3];
  uint8_t       ls_transition[LIGHT_SEGMENTS_MAX];      // 0 = On/Off, 1 = CrossFade, 2 = WipeLeft, 3=WipeRight, 4 = WipeCenter, 5 = Random

};

#ifdef __cplusplus
}
#endif


class Settings
{
  public:
    struct PersistentSettings settings;
    void load();
    void loadDelta();
    void saveAll();
    void save(byte rotate);
    void eraseAll(bool trace);
    void loadDefaults();
    char* composeSetting(char* output, const char* input, int size);
    void composeMqttTopic(char *stopic, int lenTopic, byte prefix, char *topic, const char* subtopic);
    char* getHostname(char *output, int size);
    void dumpSettings();

    byte getSerialLogLevel();
    Settings *setSerialLogLevel(byte logLevel);
    

  protected:
    void dumpSettings(char* parms);

  private:
    uint8_t stop_flash_rotate = 0;              // Allow flash configuration rotation
    byte seriallog_level;                       // Current copy of Settings.seriallog_level

    boolean parseIp(uint32_t* addr, const char* str);
    uint32_t getSettingsHash();
    void loadDefaultsSet1();
};



#endif  // _SETTINGS_H_
