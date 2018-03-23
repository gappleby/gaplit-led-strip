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
 #include <Arduino.h>

#include "StreamString.h"
#include "settings.h"

boolean Settings::parseIp(uint32_t* addr, const char* str)
{
  uint8_t *part = (uint8_t*)addr;
  byte i;

  *addr = 0;
  for (i = 0; i < 4; i++) {
    part[i] = strtoul(str, NULL, 10);        // Convert byte
    str = strchr(str, '.');
    if (str == NULL || *str == '\0') {
      break;  // No more separators, exit
    }
    str++;                                   // Point to next character after separator
  }
  return (3 == i);
}

char* Settings::getHostname(char *output, int size) {
  if (strstr(settings.hostname, "%")) {
    strlcpy(settings.hostname, WIFI_HOSTNAME, sizeof(settings.hostname));
    char mqtt_topic[33];
    composeSetting(mqtt_topic, settings.mqtt_topic, sizeof(mqtt_topic));
    snprintf_P(output, size - 1, settings.hostname, mqtt_topic, ESP.getChipId() & 0x1FFF);
  } else {
    snprintf_P(output, size - 1, settings.hostname);
  }
}

char* Settings::composeSetting(char* output, const char* input, int size)
{
  char *token;
  uint8_t digits = 0;

  if (strstr(input, "%")) {
    strlcpy(output, input, size);
    token = strtok(output, "%");
    if (strstr(input, "%") == input) {
      output[0] = '\0';
    } else {
      token = strtok(NULL, "");
    }
    if (token != NULL) {
      digits = atoi(token);
      if (digits) {
        if (strchr(token, 'd')) {
          snprintf_P(output, size, PSTR("%s%c0%dd"), output, '%', digits);
          snprintf_P(output, size, output, ESP.getChipId() & 0x1fff);       // %04d - short chip ID in dec, like in hostname
        } else {
          snprintf_P(output, size, PSTR("%s%c0%dX"), output, '%', digits);
          snprintf_P(output, size, output, ESP.getChipId());                // %06X - full chip ID in hex
        }
      } else {
        if (strchr(token, 'd')) {
          snprintf_P(output, size, PSTR("%s%d"), output, ESP.getChipId());  // %d - full chip ID in dec
          digits = 8;
        }
      }
    }
  }
  if (!digits) {
    strlcpy(output, input, size);
  }
  return output;
}

void Settings::composeMqttTopic(char *stopic, int lenTopic, byte prefix, char *topic, const char* subtopic)
{
  /* prefix 0 = Cmnd
     prefix 1 = Stat
     prefix 2 = Tele
  */
  char romram[33];
  String fulltopic;

  snprintf_P(romram, sizeof(romram), subtopic);
  {
    fulltopic = settings.mqtt_fulltopic;
    if ((0 == prefix) && (-1 == fulltopic.indexOf(F(MQTT_TOKEN_PREFIX)))) {
      fulltopic += F("/" MQTT_TOKEN_PREFIX);  // Need prefix for commands to handle mqtt topic loops
    }
    for (byte i = 0; i < 3; i++) {
      if ('\0' == settings.mqtt_prefix[i][0]) {
        snprintf_P(settings.mqtt_prefix[i], sizeof(settings.mqtt_prefix[i]), MQTT_PREFIX[i]);
      }
    }
    fulltopic.replace(F(MQTT_TOKEN_PREFIX), settings.mqtt_prefix[prefix]);
    fulltopic.replace(F(MQTT_TOKEN_TOPIC), topic);
  }
  fulltopic.replace(F("#"), "");
  fulltopic.replace(F("//"), "/");
  if (!fulltopic.endsWith("/")) {
    fulltopic += "/";
  }
  snprintf_P(stopic, lenTopic, PSTR("%s%s"), fulltopic.c_str(), romram);
}



extern "C" {
#include "spi_flash.h"
}

#include "eboot_command.h"

extern "C" uint32_t _SPIFFS_end;

// From libraries/EEPROM/EEPROM.cpp EEPROMClass
#define SPIFFS_END          ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE

#define SETTINGS_LOCATION_3 SPIFFS_END - 4
#define SETTINGS_LOCATION   SPIFFS_END  // No need for SPIFFS as it uses EEPROM area
#define CFG_ROTATES         8           // Number of flash sectors used (handles uploads)

uint32_t settings_hash = 0;
uint32_t settings_location = SETTINGS_LOCATION;

uint32_t Settings::getSettingsHash()
{
  uint32_t hash = 0;
  uint8_t *bytes = (uint8_t*)&settings;

  for (uint16_t i = 0; i < sizeof(PersistentSettings); i++) {
    hash += bytes[i] * (i + 1);
  }
  return hash;
}

void Settings::saveAll()
{
  save(0);
}


void Settings::save(byte rotate)
{
  /* Save configuration in eeprom or one of 7 slots below

     rotate 0 = Save in next flash slot
     rotate 1 = Save only in eeprom flash slot until SetOption12 0 or restart
     rotate 2 = Save in eeprom flash slot, erase next flash slots and continue depending on stop_flash_rotate
     stop_flash_rotate 0 = Allow flash slot rotation (SetOption12 0)
     stop_flash_rotate 1 = Allow only eeprom flash slot use (SetOption12 1)
  */

  if ((getSettingsHash() != settings_hash) || rotate) {
    if (1 == rotate) {   // Use eeprom flash slot only and disable flash rotate from now on (upgrade)
      stop_flash_rotate = 1;
    }
    if (2 == rotate) {   // Use eeprom flash slot and erase next flash slots if stop_flash_rotate is off (default)
      settings_location = SETTINGS_LOCATION + 1;
    }
    if (stop_flash_rotate) {
      settings_location = SETTINGS_LOCATION;
    } else {
      settings_location--;
      if (settings_location <= (SETTINGS_LOCATION - CFG_ROTATES)) {
        settings_location = SETTINGS_LOCATION;
      }
    }
    settings.save_flag++;
    noInterrupts();
    spi_flash_erase_sector(settings_location);
    spi_flash_write(settings_location * SPI_FLASH_SEC_SIZE, (uint32*)&settings, sizeof(PersistentSettings));
    interrupts();
    if (!stop_flash_rotate && rotate) {
      for (byte i = 1; i < CFG_ROTATES; i++) {
        noInterrupts();
        spi_flash_erase_sector(settings_location - i); // Delete previous configurations by resetting to 0xFF
        interrupts();
        delay(1);
      }
    }

    settings_hash = getSettingsHash();
  }

}


void Settings::load()
{
  /* Load configuration from eeprom or one of 7 slots below if first load does not stop_flash_rotate
  */
  struct _PersistentSettingsHeader {
    unsigned long cfg_holder;
    unsigned long save_flag;
  } _SettingsH;

  settings_location = SETTINGS_LOCATION + 1;
  for (byte i = 0; i < CFG_ROTATES; i++) {
    settings_location--;
    noInterrupts();
    spi_flash_read(settings_location * SPI_FLASH_SEC_SIZE, (uint32*)&settings, sizeof(PersistentSettings));
    spi_flash_read((settings_location - 1) * SPI_FLASH_SEC_SIZE, (uint32*)&_SettingsH, sizeof(_PersistentSettingsHeader));
    interrupts();

    if ((settings.cfg_holder != _SettingsH.cfg_holder) || (settings.save_flag > _SettingsH.save_flag)) {
      break;
    }
    delay(1);
  }

  if (settings.cfg_holder != CFG_HOLDER) {
    // Auto upgrade
    noInterrupts();
    spi_flash_read((SETTINGS_LOCATION_3) * SPI_FLASH_SEC_SIZE, (uint32*)&settings, sizeof(PersistentSettings));
    spi_flash_read((SETTINGS_LOCATION_3 + 1) * SPI_FLASH_SEC_SIZE, (uint32*)&_SettingsH, sizeof(_SettingsH));
    if (settings.save_flag < _SettingsH.save_flag)
      spi_flash_read((SETTINGS_LOCATION_3 + 1) * SPI_FLASH_SEC_SIZE, (uint32*)&settings, sizeof(PersistentSettings));
    interrupts();
    if ( (settings.cfg_holder != CFG_HOLDER) ) {
      loadDefaults();
    }
  }

  settings_hash = getSettingsHash();

}

void Settings::eraseAll(bool serialOutput)
{
  SpiFlashOpResult result;

  uint32_t _sectorStart = (ESP.getSketchSize() / SPI_FLASH_SEC_SIZE) + 1;
  uint32_t _sectorEnd = ESP.getFlashChipRealSize() / SPI_FLASH_SEC_SIZE;
  boolean _serialoutput = serialOutput;

  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++) {
    noInterrupts();
    result = spi_flash_erase_sector(_sector);
    interrupts();
    if (_serialoutput) {
      Serial.print(F(D_LOG_APPLICATION D_ERASED_SECTOR " "));
      Serial.print(_sector);
      if (SPI_FLASH_RESULT_OK == result) {
        Serial.println(F(" " D_OK));
      } else {
        Serial.println(F(" " D_ERROR));
      }
      delay(10);
    }
  }
}

void Settings::dumpSettings(char* parms)
{
#define CFG_COLS 16

  uint16_t idx;
  uint16_t maxrow;
  uint16_t row;
  uint16_t col;
  char *p;
  char log_data[1024];                       // Logging


  uint8_t *buffer = (uint8_t *) &settings;
  maxrow = ((sizeof(PersistentSettings) + CFG_COLS) / CFG_COLS);

  uint16_t srow = strtol(parms, &p, 16) / CFG_COLS;
  uint16_t mrow = strtol(p, &p, 10);

  if (0 == mrow) {  // Default only 8 lines
    mrow = 8;
  }
  if (srow > maxrow) {
    srow = maxrow - mrow;
  }
  if (mrow < (maxrow - srow)) {
    maxrow = srow + mrow;
  }

  for (row = srow; row < maxrow; row++) {
    idx = row * CFG_COLS;
    snprintf_P(log_data, sizeof(log_data), PSTR("%03X:"), idx);
    for (col = 0; col < CFG_COLS; col++) {
      if (!(col % 4)) {
        snprintf_P(log_data, sizeof(log_data), PSTR("%s "), log_data);
      }
      snprintf_P(log_data, sizeof(log_data), PSTR("%s %02X"), log_data, buffer[idx + col]);
    }
    snprintf_P(log_data, sizeof(log_data), PSTR("%s |"), log_data);
    for (col = 0; col < CFG_COLS; col++) {
      snprintf_P(log_data, sizeof(log_data), PSTR("%s%c"), log_data, ((buffer[idx + col] > 0x20) && (buffer[idx + col] < 0x7F)) ? (char)buffer[idx + col] : ' ');
    }
    snprintf_P(log_data, sizeof(log_data), PSTR("%s|"), log_data);
    delay(1);
  }
}

/********************************************************************************************/

void Settings::loadDefaults()
{
  loadDefaultsSet1();
  save(2);
}

void Settings::loadDefaultsSet1()
{
  memset(&settings, 0x00, sizeof(PersistentSettings));

  settings.cfg_holder = CFG_HOLDER;
  settings.version = VERSION;
  settings.save_data = SAVE_DATA;
  settings.timezone = APP_TIMEZONE;
  strlcpy(settings.ota_url, OTA_URL, sizeof(settings.ota_url));

  settings.seriallog_level = SERIAL_LOG_LEVEL;
  strlcpy(settings.hostname, WIFI_HOSTNAME, sizeof(settings.hostname));

  settings.mqtt_enabled = MQTT_ENABLED;
  strlcpy(settings.mqtt_host, MQTT_HOST, sizeof(settings.mqtt_host));
  settings.mqtt_port = MQTT_PORT;
  strlcpy(settings.mqtt_client, MQTT_CLIENT_ID, sizeof(settings.mqtt_client));
  strlcpy(settings.mqtt_user, MQTT_USER, sizeof(settings.mqtt_user));
  strlcpy(settings.mqtt_pwd, MQTT_PASS, sizeof(settings.mqtt_pwd));
  strlcpy(settings.mqtt_topic, MQTT_TOPIC, sizeof(settings.mqtt_topic));
  strlcpy(settings.button_topic, MQTT_TOPIC, sizeof(settings.button_topic));
  strlcpy(settings.mqtt_grptopic, MQTT_GRPTOPIC, sizeof(settings.mqtt_grptopic));
  strlcpy(settings.mqtt_switch_topic, MQTT_TOPIC, sizeof(settings.mqtt_switch_topic));

  settings.status_light_gpio = STATUS_GPIO_PIN_DEFAULT;

  settings.relay_gpio = RELAY_GPIO_PIN_DEFAULT;
  settings.relay_start_delay = RELAY_START_DELAY;
  settings.relay_stop_delay = RELAY_STOP_DELAY;
  

  strlcpy(settings.web_user, WEB_USERNAME, sizeof(settings.web_user));
  strlcpy(settings.web_password, WEB_PASSWORD, sizeof(settings.web_password));

  strlcpy(settings.mqtt_prefix[0], SUB_PREFIX, sizeof(settings.mqtt_prefix[0]));
  strlcpy(settings.mqtt_prefix[1], PUB_PREFIX, sizeof(settings.mqtt_prefix[1]));
  strlcpy(settings.mqtt_prefix[2], PUB_PREFIX2, sizeof(settings.mqtt_prefix[2]));
  strlcpy(settings.mqtt_state_text[0], MQTT_STATUS_OFF, sizeof(settings.mqtt_state_text[0]));
  strlcpy(settings.mqtt_state_text[1], MQTT_STATUS_ON, sizeof(settings.mqtt_state_text[1]));
  strlcpy(settings.mqtt_state_text[2], MQTT_CMND_TOGGLE, sizeof(settings.mqtt_state_text[2]));
  strlcpy(settings.mqtt_state_text[3], MQTT_CMND_HOLD, sizeof(settings.mqtt_state_text[3]));  // v5.1.6
  strlcpy(settings.mqtt_fulltopic, MQTT_FULLTOPIC, sizeof(settings.mqtt_fulltopic));
  settings.mqtt_retry = MQTT_RETRY_SECS;

  settings.ls_pixels = WS2812_LEDS;
  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    settings.ls_topicIndex[n] = 0;
    snprintf(settings.ls_displayName[n], DISPLAY_NAME_MAX_LEN - 1, "%s %d", DISPLAY_NAME_DEFAULT, n + 1);
    settings.ls_powerOnState[n] = 0;
    settings.ls_startPixel[n] = 0;
    settings.ls_endPixel[n] = 0;
    settings.ls_density[n] = 1;
    settings.ls_colourOn[n][0] = 255;
    settings.ls_colourOn[n][1] = 255;
    settings.ls_colourOn[n][2] = 255;
    settings.ls_colourOff[n][0] = 0;
    settings.ls_colourOff[n][1] = 0;
    settings.ls_colourOff[n][2] = 0;

  }
  settings.ls_tracerPixels = 0;
  settings.ls_tracerColour[0] = 255;
  settings.ls_tracerColour[1] = 255;
  settings.ls_tracerColour[2] = 255;

  settings.ls_gpio = LS_GPIO_PIN_DEFAULT;

}

/********************************************************************************************/


/********************************************************************************************/

void Settings::loadDelta()
{
  if (settings.version != VERSION) {

    if (settings.version < 0x0100000C) {
    }

    settings.version = VERSION;
    save(1);

  }
}

void Settings::dumpSettings() {
  Serial.printf("\nSettings ----------------------------\n"
                "cfg_holder : %08X \n"
                "save_flag : %08X \n"
                "version : %08X \n"
                "bootcount : %ld \n"
                "save_data : %d \n"
                "timezone : %d \n"
                "ota_url : [%s] \n"
                "hostname : [%s] \n"
                "web_user : [%s] \n"
                "web_password : [%s] \n"
                "status_light_gpio : %d \n"
                "seriallog_level : %d \n"
                "mqtt_enabled : [%d] \n"
                "mqtt_host : [%s] \n"
                "mqtt_port : %d \n"
                "mqtt_client : [%s] \n"
                "mqtt_user : [%s] \n"
                "mqtt_pwd : [%s] \n"
                "mqtt_prefix[0] : [%s] "
                "mqtt_prefix[1] : [%s] "
                "mqtt_prefix[2] : [%s] \n"
                "mqtt_topic : [%s] \n"
                "mqtt_fulltopic : [%s] \n"
                "button_topic : [%s] \n"
                "mqtt_grptopic : [%s] \n"
                "mqtt_retry : %d \n"
                "switch_topic : [%s] \n"
                "state_text[] : [%s] "
                "state_text[] : [%s] "
                "state_text[] : [%s] "
                "state_text[] : [%s] \n"
                "ls_tracerPixels : %d \n"
                "ls_tracerColour : %d %d %d \n"
                "ls_gpio : %d \n"
                "ls_pixels : %d \n",

                settings.cfg_holder,
                settings.save_flag,
                settings.version,
                settings.bootcount,

                settings.save_data,
                settings.timezone,
                settings.ota_url,
                settings.hostname,
                settings.web_user,
                settings.web_password,
                settings.status_light_gpio,
                settings.seriallog_level,
                settings.mqtt_enabled,
                settings.mqtt_host,
                settings.mqtt_port,
                settings.mqtt_client,
                settings.mqtt_user,
                settings.mqtt_pwd,
                settings.mqtt_prefix[0], settings.mqtt_prefix[1], settings.mqtt_prefix[2],
                settings.mqtt_topic,
                settings.mqtt_fulltopic,
                settings.button_topic,
                settings.mqtt_grptopic,
                settings.mqtt_retry,
                settings.mqtt_switch_topic,
                settings.mqtt_state_text[0], settings.mqtt_state_text[1], settings.mqtt_state_text[2], settings.mqtt_state_text[3],

                // Tracer Pixels
                settings.ls_tracerPixels,
                settings.ls_tracerColour[0], settings.ls_tracerColour[1], settings.ls_tracerColour[2],

                // Light Strip Settings
                settings.ls_gpio,
                settings.ls_pixels);


  for (int n = 0; n < LIGHT_SEGMENTS_MAX; n++) {
    Serial.printf(
      "\nls item : %d\n"
      "ls_displayName[] : [%s] \n"
      "ls_powerOnState : %d \n"
      "ls_topicIndex : %d \n"
      "ls_startPixel : %d \n"
      "ls_endPixel : %d \n"
      "ls_density : %d \n"
      "ls_colourOn : %d %d %d \n"
      "ls_colourOff : %d %d %d \n",
      n,
      settings.ls_displayName[n],// 3AC
      settings.ls_powerOnState[n],
      settings.ls_topicIndex[n],
      settings.ls_startPixel[n],
      settings.ls_endPixel[n],
      settings.ls_density[n],
      settings.ls_colourOn[n][0], settings.ls_colourOn[n][1], settings.ls_colourOn[n][2],
      settings.ls_colourOff[n][0], settings.ls_colourOff[n][1], settings.ls_colourOff[n][2]);
  }
}



