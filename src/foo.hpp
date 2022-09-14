#pragma once

#include <vector>

#include "rack.hpp"

using namespace rack;

//--------------------------------------------------------------
// Peak is adapted from /Rack-SDK/include/dsp/vumeter.hpp
//--------------------------------------------------------------

struct Peak {

  private:

    /** Inverse time constant in 1/seconds */
    static constexpr float lambda = 30.f;

  public:

    float value = 0.0f;

    void process(float deltaTime, float sample) {
        sample = std::fabs(sample);
        if (sample >= value) {
            value = sample;
        } else {
            value += (sample - value) * lambda * deltaTime;
        }
    }

	float getBrightness(float dbMin, float dbMax) {

        float amplitude = value/5.0f;

        // TODO: lookup table?
		float db = dsp::amplitudeToDb(amplitude);

		if (db >= dbMax)
			return 1.f;
		else if (db <= dbMin)
			return 0.f;
		else
			return math::rescale(db, dbMin, dbMax, 0.f, 1.f);
	}
};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

  public:

    float sum = 0.0f;
    Peak peak;

    void process(float deltaTime, Input& input) {

        // process each channel
        sum = 0.0f;
        int channels = std::max(input.getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = input.getPolyVoltage(ch);

            // TODO levels, mute, pan
            float out = in;

            sum += out;
        }

        // done
        peak.process(deltaTime, sum);
    }

    void disconnect(float deltaTime) {
        sum = 0.0f;
        peak.process(deltaTime, 0.0f);
    }

    void updateLeds(std::vector<Light>& leds, int ledID) {
        leds[ledID + 0].setBrightness(peak.getBrightness(  0,   3));
        leds[ledID + 1].setBrightness(peak.getBrightness( -3,   0));
        leds[ledID + 2].setBrightness(peak.getBrightness( -6,  -3));
        leds[ledID + 3].setBrightness(peak.getBrightness(-12,  -6));
        leds[ledID + 4].setBrightness(peak.getBrightness(-24, -12));
        leds[ledID + 5].setBrightness(peak.getBrightness(-36, -24));
        leds[ledID + 6].setBrightness(peak.getBrightness(-36, -36));
        leds[ledID + 7].setBrightness(peak.getBrightness(-48, -36));
    }
};

struct StereoTrack {

    MonoTrack left;
    MonoTrack right;

    void process(float deltaTime, Input& leftInput, Input& rightInput) {

        if (leftInput.isConnected()) {
            left.process(deltaTime, leftInput);
        } else {
            left.disconnect(deltaTime);
        }

        //if (rightInput.isConnected()) {
        //    right.process(deltaTime, rightInput);
        //} else {
        //    right.disconnect(deltaTime);
        //}
    }
};
