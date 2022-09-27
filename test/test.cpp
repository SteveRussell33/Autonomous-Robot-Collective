
#include <cmath>
#include <iomanip>
#include <iostream>

void dump(float v) {
    std::cout << std::fixed;
    std::cout << std::setprecision(7);
    std::cout << v;
}

//--------------------------------------------------------------
// LinearRamp
//--------------------------------------------------------------

class LinearRamp {

    float sampleRate = 1.0f;
    float time = 1.0f; // in seconds
    float divisor = 1.0f;

    float target = 0.0f;
    float increment = 0.0f;
    float value = 0.0f;

    void recalc() {
        divisor = 1.0f / (sampleRate * time);
    }

  public:

    void onSampleRateChange(float sampleRate_) {
        assert(sampleRate_ > 0.0f);
        sampleRate = sampleRate_;
        recalc();
    }

    void setTime(float time_) {
        assert(time_ > 0.0f);
        time = time_;
        recalc();
    }

    float next(float target_) {

        // done already
        if (target_ == value) {
            return value;
        }

        // new target
        if (target != target_) {
            target = target_;
            increment = (target - value) * divisor;
        }

        // increment the value
        bool rising = (target > value);
        value += increment;

        // rising
        if (rising) {
            if (value > target) {
                value = target;
            }
        }
        // falling
        else {
            if (value < target) {
                value = target;
            }
        }

        return value;
    }
};

void testLinearRamp() {

    LinearRamp ramp;
    ramp.setTime(0.005f);
    ramp.onSampleRateChange(48000.0f);

    for (int t = 0; t < 300; t++) {
        std::cout << t << ": ";
        float value = ramp.next(2.0f);
        dump(value);
        std::cout << std::endl;
    }

    std::cout << "-------------------" << std::endl;

    for (int t = 0; t < 300; t++) {
        std::cout << t << ": ";
        float value = ramp.next(1.234567f);
        dump(value);
        std::cout << std::endl;
    }
}

int main() {
    testLinearRamp();
}
