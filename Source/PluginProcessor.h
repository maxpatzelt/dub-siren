#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/DubOscillator.h"
#include "DSP/LFO.h"
#include "DSP/DubDelay.h"

/**
 * Dub Siren VST Processor
 *
 * Classic dub siren synthesizer with:
 * - Gritty square wave VCO
 * - Dub-style delay effect
 * - Two LFOs for modulation routing
 */
class SimpleSynthProcessor : public juce::AudioProcessor
{
public:
    SimpleSynthProcessor();
    ~SimpleSynthProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter access
    juce::AudioProcessorValueTreeState& getParameters() { return parameters_; }

    // LFO routing enums
    enum class LFO1Target {
        None = 0,
        VCORate,
        DelayTime,
        DelayFeedback
    };

    enum class LFO2Target {
        None = 0,
        LFO1Rate,
        LFO1Amount,
        DelayWetDry
    };

private:
    void updateDSPFromParameters();

    // DSP modules
    SimpleSynth::DSP::DubOscillator dubOscillator_;
    SimpleSynth::DSP::LFO lfo1_;
    SimpleSynth::DSP::LFO lfo2_;
    SimpleSynth::DSP::DubDelay dubDelay_;

    // Parameter management
    juce::AudioProcessorValueTreeState parameters_;

    // MIDI state
    int currentMidiNote_ = -1;
    bool isNoteOn_ = false;
    float currentNoteVelocity_ = 1.0f;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthProcessor)
};
