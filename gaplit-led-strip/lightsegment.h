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
#ifndef __LightSegment_H_
#define __LightSegment_H_

using namespace std;

typedef enum _LightSegmentExpiredState { INIT, ACTIVE, RETIRED } LightSegmentExpiredState;
#define MAX_TRANSITIONS_STATES  100

typedef struct _CRGB {
  byte r;  byte b;  byte g;
} CRGB;


class LightSegment {

  public:

    const int DISABLED_LIGHT_SEGMENT_MQTT_ID = -1;
    const int DISABLED_LIGHT_SEGMENT_INDEX = -1;

    // turn the light on / off
    virtual void setLightState(bool isOn) = 0;

    bool getLightState() {
      return isOn;
    }

    // Configure light segment parameters
    int getNumPixels() {
      return numPixels;
    }
    int getStartPixel() {
      return startPixel;
    }

    int getEndPixel() {
      return endPixel;
    }

    int getDensity() {
      return density;
    }

    auto setDensity(int density) {
      this->density = density;
      return this;
    }

    auto setNumPixels(int n) {
      numPixels = n;
      reset();
      return this;
    }
    auto setStartPixel(int n) {
      startPixel = n;
      reset();
      return this;
    }
    auto setEndPixel(int n) {
      endPixel = n;
      reset();
      return this;
    }
    auto setStartEndPixel(int startPixel, int endPixel) {
      this->startPixel = startPixel;
      this->endPixel = endPixel;

      // Set the number of pixels and correct the endPixel count.
      int numPixels = endPixel - startPixel + 1;
      if (numPixels < 0) numPixels = 0;
      this->endPixel = startPixel + numPixels - 1;

      setNumPixels(numPixels);
      return this;
    }

    // Call reset method after setting the parameters for the light to reload
    // the settings.  Light will default off.
    void reset() {

      if (pixels) {
        delete [] pixels;
        pixels = NULL;
      }

      pixels = new CRGB[numPixels];

      updateRequired = true;

    }

    virtual bool update() = 0;

    // Get / Set Individual Pixel
    virtual CRGB getPixel(int n) = 0;

    void setPixel(int n , CRGB &p)
    {
      if (pixels == NULL) {
        return;
      }

      if (n >= numPixels || n < 0) {
        return;
      }

      pixels[n] = p;
    }

    LightSegment() :
      pixels(NULL), startPixel(0), endPixel(0), numPixels(0), density(1),
      isOn(false), updateRequired(true), state(INIT), debug_serial_output(false),
      index(DISABLED_LIGHT_SEGMENT_INDEX), mqttId(DISABLED_LIGHT_SEGMENT_MQTT_ID) {
      if (debug_serial_output) Serial.printf("\nLightSegment::LightSegment : lightIndex=%d", index);

    }

    ~LightSegment() {
      if (debug_serial_output) Serial.printf("\nLightSegment::~LightSegment : lightIndex=%d", index);

      if (pixels) delete [] pixels;
    }

    auto setState(LightSegmentExpiredState state) {
      this->state = state;
      return this;
    }
    LightSegmentExpiredState getState() {
      return state;
    }
    bool isActive() {
      return (state == ACTIVE);
    }

    auto setIndex(int index) {
      this->index = index;
      return this;
    }
    int getIndex() {
      return index;
    }

    auto setMqttId(int mqttId) {
      this->mqttId = mqttId;
      return this;
    }
    int getMqttId() {
      return mqttId;
    }

    auto setSerialDebug(bool debugOn) {
      debug_serial_output = debugOn;
      return this;
    }

    LightSegment(const LightSegment& rhs) {
      state = rhs.state; numPixels = rhs.numPixels; startPixel = rhs.startPixel; endPixel = rhs.endPixel; density = rhs.density;
      isOn = rhs.isOn; updateRequired = rhs.updateRequired; index = rhs.index; mqttId = rhs.mqttId;

      if (rhs.pixels == NULL) {
        pixels = NULL;
      } else {
        pixels = new CRGB[numPixels];
        for (int n = 0; n < numPixels; n++) {
          pixels[n] = rhs.pixels[n];
        }
      }
      if (debug_serial_output) Serial.printf("\nLightSegment::LightSegment(LightSegment&) : lightIndex=%d", index);

    }
    LightSegment& operator=(const LightSegment& rhs) {};

  protected:
    LightSegmentExpiredState state;
    int numPixels;
    int startPixel;
    int endPixel;
    bool isOn;
    bool updateRequired;
    int index;
    int mqttId;
    int density;
    bool debug_serial_output;

    CRGB *pixels = NULL;

};

#endif  // __LightSegment_H_
