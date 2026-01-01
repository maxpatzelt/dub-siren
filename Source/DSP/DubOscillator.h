#pragma once

#include "Common.h"

namespace SimpleSynth {
namespace DSP {

/**
 * Dub Siren Oscillator
 *
 * Classic gritty square wave with analog character.
 * Generates the main siren tone with slight harmonic instability.
 */
class DubOscillator {
public:
    DubOscillator();
    ~DubOscillator() = default;

    void Init(float sampleRate);
    void SetFrequency(float frequency);
    void SetLevel(float level); // 0.0 to 1.0
    void Reset();

    float ProcessSample();
    void Process(float* output, size_t numSamples);

private:
    float GenerateSquareWave();

    float sampleRate_;
    float frequency_;
    float level_;
    float phase_;
    float phaseIncrement_;

    // Analog drift simulation
    float driftPhase_;
    float driftAmount_;
};

} // namespace DSP
} // namespace SimpleSynth
