#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/DubOscillator.h"
#include "DSP/LFO.h"
#include "DSP/DubDelay.h"
#include "DSP/Envelope.h"

/**
 * Dub Siren VST Processor  v2.0
 *
 * Classic dub siren synthesizer with:
 *  - Band-limited VCO (Square / Saw / Triangle)  with polyBLEP
 *  - Portamento / glide
 *  - Two LFOs (Sine / Tri / Square / S+H)  with full modulation routing
 *  - Dub-style delay effect
 *  - Stereo output
 *
 * Audio thread rules:
 *  - Zero heap allocation in processBlock
 *  - All APVTS parameter reads cached ONCE at block boundary
 *  - No rand() — xorshift32 PRNG used in DSP classes
 */
class DubSirenProcessor : public juce::AudioProcessor
{
public:
    DubSirenProcessor();
    ~DubSirenProcessor() override;

    //==========================================================================
    void prepareToPlay  (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock   (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool  acceptsMidi()   const override;
    bool  producesMidi()  const override;
    bool  isMidiEffect()  const override;
    double getTailLengthSeconds() const override;

    int   getNumPrograms() override;
    int   getCurrentProgram() override;
    void  setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void  changeProgramName(int, const juce::String&) override;

    void  getStateInformation(juce::MemoryBlock&) override;
    void  setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }

    // LFO routing enums (kept public so the editor can label combo items correctly)
    enum class LFO1Target { None = 0, VCORate, DelayTime, DelayFeedback };
    enum class LFO2Target { None = 0, LFO1Rate, LFO1Amount, DelayWetDry };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP modules
    DubSiren::DSP::DubOscillator dubOscillator_;
    DubSiren::DSP::LFO           lfo1_;
    DubSiren::DSP::LFO           lfo2_;
    DubSiren::DSP::DubDelay      dubDelay_;
    DubSiren::DSP::Envelope      envelope_;

    // Parameter management
    juce::AudioProcessorValueTreeState parameters_;

    // MIDI state
    int   currentMidiNote_    = -1;
    bool  isNoteOn_           = false;
    float currentNoteVelocity_= 1.0f;

    // Portamento (1-pole slew on VCO frequency)
    float slewedVCOFreq_      = 440.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DubSirenProcessor)
};
