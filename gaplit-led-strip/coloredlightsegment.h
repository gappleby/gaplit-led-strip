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
#ifndef __ColoredLightSegment_H_
#define __ColoredLightSegment_H_

using namespace std;

#include "lightsegment.h"

const CRGB white = { 0xff, 0xff, 0xff};
const CRGB black = { 0x0, 0x0, 0x0};

class ColoredLightSegment : public LightSegment {

  public:
    auto setPixelOnColor(CRGB color) {
      pixelOn = color;
      return this;
    }
    auto setPixelOffColor(CRGB color) {
      pixelOff = color;
      return this;
    }

    CRGB getPixelOnColor() {
      return pixelOn;
    }
    CRGB getPixelOffColor() {
      return pixelOff;
    }

    auto setTransitionType(uint8_t transitionType) {
      if (transitionType > 6) transitionType = 0; //default to on/off

      this->transitionType = transitionType;
      this->transitionState = 0;
      updateRequired = true;

      return this;
    }

    uint8_t getTransitionType() {
      return transitionType;
    }

    // turn the light on / off
    void setLightState(bool isOn) {

      if (this->isOn != isOn) {
        if (updateRequired) {
          // Invert the transition
          transitionState = MAX_TRANSITIONS_STATES - transitionState;
        } else {
          updateRequired = true;
          transitionState = MAX_TRANSITIONS_STATES + 1;
        }
        this->isOn = isOn;
      }

      for (int n = 0; n < numPixels; n++) {
        pixels[n] = isOn ? pixelOn : pixelOff;
      }

      if (debug_serial_output) Serial.printf("\nColoredLightSegment->setLightState : lightIndex=%d currentState=%s", index, isOn ? "on" : "off");

    }

    // Call to trigger the next animation state update for the light segment.
    // If return false, then no pixels were updated.
    bool update() {
      if (debug_serial_output) Serial.printf("\nColoredLightSegment::serial : updateRequired=%s  transitionType=%d", updateRequired ? "true" : "false", transitionType);

      if (!updateRequired) {
        return false;
      }

      if (pixels == NULL) return true;

      if (transitionType == 0) {
        transitionState = 0;
      }

      if (transitionState > 0) {
        transitionState--;
        updateRequired = true;
      } else {
        updateRequired = false;
      }
      return true;
    }

    CRGB getPixel(int n)
    {
      CRGB nullPixel = {0x00, 0x00, 0x00};
      if (pixels == NULL) {
        return nullPixel;
      }

      if (n >= numPixels || n < 0) {
        return nullPixel;
      }

      double scale = 1.0;
      int incTransitionState = MAX_TRANSITIONS_STATES - transitionState;
      CRGB p = pixels[n];

      switch (transitionType) {
        case 0: // 0 = On/Off
          p  = isOn ? pixelOn : pixelOff;
          break;

        case 1: // 1 = cross fade
          scale = (1.0 * incTransitionState / MAX_TRANSITIONS_STATES);
          if (isOn) {
            p.r = abs(pixelOff.r + (scale * ( pixelOn.r - pixelOff.r )) );
            p.g = abs(pixelOff.g + (scale * ( pixelOn.g - pixelOff.g )) );
            p.b = abs(pixelOff.b + (scale * ( pixelOn.b - pixelOff.b )) );
          }
          else {
            p.r = abs(pixelOn.r - (scale * ( pixelOn.r - pixelOff.r )) );
            p.g = abs(pixelOn.g - (scale * ( pixelOn.g - pixelOff.g )) );
            p.b = abs(pixelOn.b - (scale * ( pixelOn.b - pixelOff.b )) );
          }

          break;

        case 2: // 2 = WipeLeft

          if (isOn) {
            scale = 1.0 * incTransitionState / MAX_TRANSITIONS_STATES * numPixels;
            p = (n < scale) ? pixelOn : pixelOff;
          } else {
            scale = 1.0 * transitionState / MAX_TRANSITIONS_STATES * numPixels;
            p = (n < scale)  ? pixelOn : pixelOff;
          }
          break;

        case 3: // 3 = WipeRight

          if (isOn) {
            scale = 1.0 * incTransitionState / MAX_TRANSITIONS_STATES * numPixels;
            p = ( (numPixels - n - 1) < scale) ? pixelOn : pixelOff;
          } else {
            scale = 1.0 * transitionState / MAX_TRANSITIONS_STATES * numPixels;
            p = ( (numPixels - n - 1) < scale)  ? pixelOn : pixelOff;
          }
          break;

        case 4: // 4 = WipeCenter

          if (isOn) {
            scale = 1.0 * incTransitionState / MAX_TRANSITIONS_STATES * numPixels / 2.0;
            p = ((n >= ((numPixels / 2) - scale)) && (n <= ((numPixels / 2) + scale))) ? pixelOn : pixelOff;
          } else {
            scale = 1.0 * transitionState / MAX_TRANSITIONS_STATES * numPixels / 2.0;
            p = ((n > ((numPixels / 2) - scale)) && (n <= ((numPixels / 2) + scale))) ? pixelOn : pixelOff;
          }
          break;

        default:
          break;
      }


      if (debug_serial_output) {
        int iScale = 100.0 * scale;
        Serial.printf("\nColoredLightSegment::getPixel : pixel=%d rgb=%d|%d|%d  transitionState=%d  scale=%d  heap=%u", n, p.r, p.g, p.b, incTransitionState, iScale,  ESP.getFreeHeap());
      }

      return p;

    }


    ColoredLightSegment() : LightSegment(), pixelOn(white), pixelOff(black), transitionType(0), transitionState(0) {
      if (debug_serial_output) Serial.printf("\nColoredLightSegment::ColoredLightSegment : lightIndex=%d", index);
    }
    ~ColoredLightSegment() {
      if (debug_serial_output) Serial.printf("\nColoredLightSegment::~ColoredLightSegment : lightIndex=%d", index);
    }


    ColoredLightSegment(const ColoredLightSegment & rhs) {
      state = rhs.state; numPixels = rhs.numPixels; startPixel = rhs.startPixel; endPixel = rhs.endPixel; density = rhs.density;
      isOn = rhs.isOn; updateRequired = rhs.updateRequired; index = rhs.index; mqttId = rhs.mqttId;
      transitionType = rhs.transitionType; transitionState = rhs.transitionState;
      pixelOn = rhs.pixelOn; pixelOff = rhs.pixelOff;

      if (rhs.pixels == NULL) {
        pixels = NULL;
      } else {
        pixels = new CRGB[numPixels];
        for (int n = 0; n < numPixels; n++) {
          pixels[n] = rhs.pixels[n];
        }
      }

      if (debug_serial_output) Serial.printf("\nColoredLightSegment::ColoredLightSegment(LightSegment&) : lightIndex=%d", index);

    }
    ColoredLightSegment& operator=(const ColoredLightSegment & rhs) {};

  protected:
    CRGB pixelOn;
    CRGB pixelOff;
    uint8_t transitionType;
    int transitionState;
};


class BlackWhiteLightSegment : public ColoredLightSegment {

  public:


    CRGB getPixel(int n)
    {
      CRGB nullPixel = {0x00, 0x00, 0x00};
      if (pixels == NULL) {
        return nullPixel;
      }

      if (n >= numPixels || n < 0) {
        return nullPixel;
      }

      return isOn ? white : black;
    }

    // Call to trigger the next animation state update for the light segment.
    // If return false, then no pixels were updated.
    bool update() {

      if (debug_serial_output) Serial.printf("\nBlackWhiteLightSegment::serial : updateRequired=%s", updateRequired ? "true" : "false");

      if (!updateRequired) {
        return false;
      }

      if (pixels == NULL) return true;

      updateRequired = false;
      return true;
    }

    BlackWhiteLightSegment() : ColoredLightSegment() {
      if (debug_serial_output) Serial.printf("\nBlackWhiteLightSegment::BlackWhiteLightSegment : lightIndex=%d", index);
      pixelOn = white;
      pixelOff = black;
    }
    ~BlackWhiteLightSegment() {
      if (debug_serial_output) Serial.printf("\nBlackWhiteLightSegment::~BlackWhiteLightSegment : lightIndex=%d", index);

    }

    BlackWhiteLightSegment(const BlackWhiteLightSegment& rhs) {
      state = rhs.state; numPixels = rhs.numPixels; startPixel = rhs.startPixel; endPixel = rhs.endPixel; density = rhs.density;
      isOn = rhs.isOn; updateRequired = rhs.updateRequired; index = rhs.index; mqttId = rhs.mqttId;
      pixelOn = rhs.pixelOn; pixelOff = rhs.pixelOff;

      if (rhs.pixels == NULL) {
        pixels = NULL;
      } else {
        pixels = new CRGB[numPixels];
        for (int n = 0; n < numPixels; n++) {
          pixels[n] = rhs.pixels[n];
        }
      }

      if (debug_serial_output) Serial.printf("\nBlackWhiteLightSegment::BlackWhiteLightSegment(LightSegment&) : lightIndex=%d", index);

    }
    BlackWhiteLightSegment& operator=(const BlackWhiteLightSegment& rhs) {};

  protected:

};

#endif  // __ColoredLightSegment_H_
