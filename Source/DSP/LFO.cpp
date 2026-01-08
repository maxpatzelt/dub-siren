#include "LFO.h"
#include <cassert>
#include <cmath>

namespace SimpleSynth {
namespace DSP {

LFO::LFO()
    : sampleRate_(44100.0f)
    , rate_(1.0f)
    , amount_(0.5f)
    , phase_(0.0f)
    , phaseIncrement_(0.0f)
{
}

void LFO::Init(float sampleRate) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    sampleRate_ = sampleRate;
    phaseIncrement_ = rate_ / sampleRate_;
    phase_ = 0.0f;
}

void LFO::SetRate(float rateHz) {
    rate_ = Clamp(rateHz, 0.01f, 80.0f); // LFO range 0.01Hz to 80Hz
    phaseIncrement_ = rate_ / sampleRate_;
}

void LFO::SetAmount(float amount) {
    amount_ = Clamp(amount, 0.0f, 1.0f);
}

void LFO::Reset() {
    phase_ = 0.0f;
}

float LFO::ProcessSample() {
    // Generate sine wave LFO
    float lfoValue = std::sin(kTwoPi * phase_);

    // Advance phase
    phase_ += phaseIncrement_;
    phase_ = WrapPhase(phase_);

    return lfoValue; // Bipolar -1.0 to +1.0
}

float LFO::GetModulationValue() const {
    // Current LFO value scaled by amount
    float lfoValue = std::sin(kTwoPi * phase_);
    return lfoValue * amount_;
}

} // namespace DSP
} // namespace SimpleSynth
