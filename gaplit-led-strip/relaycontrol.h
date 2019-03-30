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
#ifndef __RelayControl_H_
#define __RelayControl_H_

class RelayControl {

  public:

    RelayControl *setStartDelayTime(int delayMs) {
      this->startDelayMs = delayMs;
      return this;
    }
    int getStartDelayTime() {
      return startDelayMs;
    }
    RelayControl *setEndDelayTime(int delayMs) {
      this->endDelayMs = delayMs;
      return this;
    }
    int getEndDelayTime() {
      return endDelayMs;
    }

    RelayControl *setGpioPin(int gpioPin) {
      this->gpioPin = gpioPin;
      if (gpioPin != NOT_SET) {
        pinMode(gpioPin, OUTPUT);
      }
      return this;
    }
    int getGpioPin() {
      return gpioPin;
    }


    const int NOT_SET = -1;


    RelayControl() {

    }

    void turnOn() {

      uint32_t now = millis();

      if (startTime != endTime && state) {
        return;
      }

      startTime = now;
      endTime = now + startDelayMs;
      if (endTime < startTime) {
        if (debug_serial_output) Serial.printf("\nRelay : Set to relay turn on time to max");
        endTime = -1; // Rollback to last value
      }
      state = true;

      if (gpioPin != NOT_SET) {
        digitalWrite(gpioPin, HIGH);
      }

      if (debug_serial_output) Serial.printf("\nRelay : ON, then delay");
      if (debug_serial_output) Serial.printf("\nRelay : startTime %u endTime %u", startTime, endTime);
    }

    void turnOff() {
      uint32_t now = millis();

      if (startTime != endTime && !state) {
        return;
      }

      startTime = now;
      endTime = now + endDelayMs;
      if (endTime < startTime) {
        if (debug_serial_output) Serial.printf("\nRelay : Set to relay turn off time to max");
        endTime = -1; // Rollback to last value
      }
      state = false;
      if (debug_serial_output) Serial.printf("\nRelay : Turn Off start=%u end=%u state=%s", startTime, endTime, state ? "true" : "false");
    }

    bool update() {
      if (startTime == endTime) {
        return getRelayState();
      }

      uint32_t now = millis();

      if (startTime <= now && now <= endTime) {
        if (debug_serial_output) Serial.printf("\nRelay : In delay");
        return getRelayState();
      }

      if (debug_serial_output) Serial.printf("\nRelay : Delay Complete start=%u end=%u now=%ul state=%s", startTime, endTime, millis(), state ? "true" : "false");

      startTime = endTime;

      if (!state) {
        if (gpioPin != NOT_SET) {
          digitalWrite(gpioPin, LOW);
        }
        if (debug_serial_output) Serial.printf("\nRelay : OFF, after the delay");
        relayState = false;
      } else {
        relayState = true;
      }

      return getRelayState();

    }

    bool getRelayState() {
      return relayState;
    }

    bool isPowerSteady() {
      if (gpioPin == NOT_SET) {
        return true;
      }
      return relayState;
    }

    RelayControl *setSerialDebug(bool debugOn) {
      debug_serial_output = debugOn;
      return this;
    }

  protected:
    bool debug_serial_output = false;
    int startDelayMs = 0;
    int endDelayMs = 0;
    int gpioPin = NOT_SET;
    uint32_t startTime = 0;
    uint32_t endTime = 0;
    bool state = false;
    bool relayState = false;
};

#endif //  __RelayControl_H_


