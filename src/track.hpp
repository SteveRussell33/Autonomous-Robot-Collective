#pragma once

#include "rack.hpp"

#include "bogaudio/dsp/filters/utility.hpp"
#include "bogaudio/dsp/signal.hpp"

#include "vu.hpp"

using namespace rack;

//--------------------------------------------------------------
// MonoTrack
//--------------------------------------------------------------

struct MonoTrack {

	void copyVoltages(Input& input) {
		for (int c = 0; c < channels; c++) {
			voltages[c] = input.getVoltage(c);
		}
	}

	void sumVoltages() {
		sum = 0.f;
		for (int c = 0; c < channels; c++) {
			sum += voltages[c];
		}
	}

  public:

    int channels = 0;
    float voltages[engine::PORT_MAX_CHANNELS] = {};
    float sum = 0.0f;
    VuLevel vuLevel;

    void onSampleRateChange(float sampleRate) {
        vuLevel.onSampleRateChange(sampleRate);
    }

    void amplify(Input& input) {

        // copy from input
        channels = input.channels;
        copyVoltages(input);

        // fader
        // TODO

        // fader CV
        // TODO

        // create summed voltage
        sumVoltages();

        // update vu level
        vuLevel.process(sum);
    }

    void disconnect() {
        channels = 0;
        sum = 0.0f;
        vuLevel.process(0.0f);
    }
};

//--------------------------------------------------------------
// StereoTrack
//--------------------------------------------------------------

struct StereoTrack {

  public:

    MonoTrack left;
    MonoTrack right;

    void onSampleRateChange(float sampleRate) {
        left.onSampleRateChange(sampleRate);
        right.onSampleRateChange(sampleRate);
    }

    void process(Input& leftInput, Input& rightInput) {

        // left connected
        if (leftInput.isConnected()) {
            left.amplify(leftInput);
        }
        // left disconnected
        else {
            left.disconnect();
        }
    }
};
