#include "DubDelay.h"
#include <cassert>
#include <cmath>
#include <algorithm>

namespace SimpleSynth {
namespace DSP {

DubDelay::DubDelay()
    : sampleRate_(44100.0f)
    , bufferSize_(0)
    , writeIndex_(0)
    , delayTimeSeconds_(0.25f)
    , feedback_(0.5f)
    , wetDry_(0.3f)
    , wobblePhase_(0.0f)
    , wobbleAmount_(0.0005f)
{
}

void DubDelay::Init(float sampleRate, float maxDelayTimeSeconds) {
    assert(sampleRate > 0.0f && "Sample rate must be positive");
    assert(maxDelayTimeSeconds > 0.0f && "Max delay time must be positive");

    sampleRate_ = sampleRate;
    bufferSize_ = static_cast<size_t>(sampleRate * maxDelayTimeSeconds);

    delayBuffer_.resize(bufferSize_);
    Reset();
}

void DubDelay::SetDelayTime(float timeSeconds) {
    delayTimeSeconds_ = Clamp(timeSeconds, 0.001f, 2.0f);
}

void DubDelay::SetFeedback(float feedback) {
    feedback_ = Clamp(feedback, 0.0f, 0.95f); // Limit to prevent runaway
}

void DubDelay::SetWetDry(float wetDry) {
    wetDry_ = Clamp(wetDry, 0.0f, 1.0f);
}

void DubDelay::Reset() {
    std::fill(delayBuffer_.begin(), delayBuffer_.end(), 0.0f);
    writeIndex_ = 0;
    wobblePhase_ = 0.0f;
}

float DubDelay::ProcessSample(float input) {
    if (bufferSize_ == 0) return input;

    // Add subtle analog wobble to delay time
    wobblePhase_ += 0.0003f;
    float wobble = std::sin(wobblePhase_ * kTwoPi) * wobbleAmount_;

    float modulatedDelayTime = delayTimeSeconds_ * (1.0f + wobble);
    modulatedDelayTime = Clamp(modulatedDelayTime, 0.001f, 2.0f);

    // Calculate read position
    size_t delaySamples = static_cast<size_t>(modulatedDelayTime * sampleRate_);
    delaySamples = std::min(delaySamples, bufferSize_ - 1);

    size_t readIndex = (writeIndex_ + bufferSize_ - delaySamples) % bufferSize_;

    // Read delayed sample
    float delayedSample = delayBuffer_[readIndex];

    // Write new sample with feedback
    delayBuffer_[writeIndex_] = input + (delayedSample * feedback_);

    // Advance write pointer
    writeIndex_ = (writeIndex_ + 1) % bufferSize_;

    // Mix wet/dry
    float dryLevel = 1.0f - wetDry_;
    float wetLevel = wetDry_;

    return (input * dryLevel) + (delayedSample * wetLevel);
}

void DubDelay::Process(float* buffer, size_t numSamples) {
    assert(buffer != nullptr && "Buffer cannot be null");

    for (size_t i = 0; i < numSamples; ++i) {
        buffer[i] = ProcessSample(buffer[i]);
    }
}

} // namespace DSP
} // namespace SimpleSynth
