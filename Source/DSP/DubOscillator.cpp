#include "DubOscillator.h"
#include <cassert>
#include <cmath>

namespace SimpleSynth {
namespace DSP {

DubOscillator::DubOscillator()
    : sampleRate_(44100.0f)
    , frequency_(440.0f)
    , level_(0.8f)
    , phase_(0.0f)
    , phaseIncrement_(0.0f)
    , driftPhase_(0.0f)
    , driftAmount_(0.002f) // Subtle analog drift
{
}

void DubOscillator::Init(float sampleRate) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    sampleRate_ = sampleRate;
    phaseIncrement_ = frequency_ / sampleRate_;
    phase_ = 0.0f;
    driftPhase_ = 0.0f;
}

void DubOscillator::SetFrequency(float frequency) {
    frequency_ = Clamp(frequency, kMinFrequency, kMaxFrequency);
    phaseIncrement_ = frequency_ / sampleRate_;
}

void DubOscillator::SetLevel(float level) {
    level_ = Clamp(level, 0.0f, 1.0f);
}

void DubOscillator::Reset() {
    phase_ = 0.0f;
    driftPhase_ = 0.0f;
}

float DubOscillator::GenerateSquareWave() {
    // Add subtle analog drift for gritty character
    driftPhase_ += 0.0001f;
    float drift = std::sin(driftPhase_) * driftAmount_;

    float modPhase = phase_ + drift;
    modPhase = WrapPhase(modPhase);

    // Square wave with slight softening
    float square = (modPhase < 0.5f) ? 1.0f : -1.0f;

    // Add tiny bit of noise for analog character
    float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.01f;

    return (square + noise) * level_;
}

float DubOscillator::ProcessSample() {
    float sample = GenerateSquareWave();

    phase_ += phaseIncrement_;
    phase_ = WrapPhase(phase_);

    return sample;
}

void DubOscillator::Process(float* output, size_t numSamples) {
    assert(output != nullptr && "Output buffer cannot be null");

    for (size_t i = 0; i < numSamples; ++i) {
        output[i] = ProcessSample();
    }
}

} // namespace DSP
} // namespace SimpleSynth
