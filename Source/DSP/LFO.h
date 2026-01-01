#pragma once

#include "Common.h"

namespace SimpleSynth {
namespace DSP {

/**
 * Low Frequency Oscillator
 *
 * Free-running modulation source.
 * Generates sine wave LFO signal for parameter modulation.
 */
class LFO {
public:
    LFO();
    ~LFO() = default;

    void Init(float sampleRate);
    void SetRate(float rateHz); // LFO frequency in Hz
    void SetAmount(float amount); // Modulation depth 0.0 to 1.0
    void Reset();

    float ProcessSample(); // Returns bipolar value -1.0 to +1.0
    float GetModulationValue() const; // Returns scaled by amount

private:
    float sampleRate_;
    float rate_;
    float amount_;
    float phase_;
    float phaseIncrement_;
};

} // namespace DSP
} // namespace SimpleSynth
