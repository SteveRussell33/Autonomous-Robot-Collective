#pragma once

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
        }
        else {
            value += (sample - value) * lambda * deltaTime;
        }
	}
};

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

private:
    int channels = 0;
    float voltages[engine::PORT_MAX_CHANNELS] = {};

	void sumVoltages() {
		sum = 0.f;
		for (int ch = 0; ch < channels; ch++) {
			sum += voltages[ch];
		}
	}

public:

    Peak peak;
    float sum = 0.0f;

    void process(float deltaTime, Input& input) {

        // process each channel
        channels = std::max(input.getChannels(), 1);
        for (int ch = 0; ch < channels; ch++) {
            float in = input.getPolyVoltage(ch);

            // TODO levels, mute, pan
            float out = in; 

            voltages[ch] = out;
        }

        // done
        sumVoltages();
        peak.process(deltaTime, sum);
    }

    void disconnect(float deltaTime) {
        channels = 0;
        sum = 0.0f;
        peak.process(deltaTime, 0.0f);
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
