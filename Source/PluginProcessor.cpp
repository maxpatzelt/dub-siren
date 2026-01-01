#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SimpleSynthProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // VCO Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoRate", "VCO Rate",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 0.1f, 0.3f), 440.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoLevel", "VCO Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    // Delay Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f), 0.375f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f), 0.6f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayWetDry", "Delay Wet/Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));

    // LFO 1 Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Rate", "LFO 1 Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.3f), 2.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Amount", "LFO 1 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo1Target", "LFO 1 Target",
        juce::StringArray{"None", "VCO Rate", "Delay Time", "Delay Feedback"}, 0));

    // LFO 2 Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Rate", "LFO 2 Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.3f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Amount", "LFO 2 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo2Target", "LFO 2 Target",
        juce::StringArray{"None", "LFO1 Rate", "LFO1 Amount", "Delay Wet/Dry"}, 0));

    return layout;
}

//==============================================================================
SimpleSynthProcessor::SimpleSynthProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::mono(), true))
    , parameters_(*this, nullptr, "DubSiren", createParameterLayout())
{
}

SimpleSynthProcessor::~SimpleSynthProcessor()
{
}

//==============================================================================
const juce::String SimpleSynthProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSynthProcessor::acceptsMidi() const
{
    return true;
}

bool SimpleSynthProcessor::producesMidi() const
{
    return false;
}

bool SimpleSynthProcessor::isMidiEffect() const
{
    return false;
}

double SimpleSynthProcessor::getTailLengthSeconds() const
{
    return 2.0; // Delay tail
}

int SimpleSynthProcessor::getNumPrograms()
{
    return 1;
}

int SimpleSynthProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleSynthProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SimpleSynthProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SimpleSynthProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SimpleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    // Initialize DSP modules
    dubOscillator_.Init(static_cast<float>(sampleRate));
    lfo1_.Init(static_cast<float>(sampleRate));
    lfo2_.Init(static_cast<float>(sampleRate));
    dubDelay_.Init(static_cast<float>(sampleRate), 2.0f);

    updateDSPFromParameters();
}

void SimpleSynthProcessor::releaseResources()
{
}

bool SimpleSynthProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono output only (classic dub siren)
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void SimpleSynthProcessor::updateDSPFromParameters()
{
    // Get parameter values
    float vcoRate = parameters_.getRawParameterValue("vcoRate")->load();
    float vcoLevel = parameters_.getRawParameterValue("vcoLevel")->load();

    float delayTime = parameters_.getRawParameterValue("delayTime")->load();
    float delayFeedback = parameters_.getRawParameterValue("delayFeedback")->load();
    float delayWetDry = parameters_.getRawParameterValue("delayWetDry")->load();

    float lfo1Rate = parameters_.getRawParameterValue("lfo1Rate")->load();
    float lfo1Amount = parameters_.getRawParameterValue("lfo1Amount")->load();
    int lfo1TargetIndex = parameters_.getRawParameterValue("lfo1Target")->load();

    float lfo2Rate = parameters_.getRawParameterValue("lfo2Rate")->load();
    float lfo2Amount = parameters_.getRawParameterValue("lfo2Amount")->load();
    int lfo2TargetIndex = parameters_.getRawParameterValue("lfo2Target")->load();

    // Update LFO 2 (it can modulate LFO 1)
    lfo2_.SetRate(lfo2Rate);
    lfo2_.SetAmount(lfo2Amount);

    float lfo2Value = lfo2_.ProcessSample();
    float lfo2Mod = lfo2Value * lfo2Amount;

    // Apply LFO2 modulation
    LFO2Target lfo2Target = static_cast<LFO2Target>(lfo2TargetIndex);

    if (lfo2Target == LFO2Target::LFO1Rate) {
        float modulatedLFO1Rate = lfo1Rate * (1.0f + lfo2Mod * 2.0f);
        lfo1Rate = juce::jlimit(0.01f, 20.0f, modulatedLFO1Rate);
    } else if (lfo2Target == LFO2Target::LFO1Amount) {
        lfo1Amount = juce::jlimit(0.0f, 1.0f, lfo1Amount + lfo2Mod * 0.5f);
    } else if (lfo2Target == LFO2Target::DelayWetDry) {
        delayWetDry = juce::jlimit(0.0f, 1.0f, delayWetDry + lfo2Mod * 0.3f);
    }

    // Update LFO 1
    lfo1_.SetRate(lfo1Rate);
    lfo1_.SetAmount(lfo1Amount);

    float lfo1Value = lfo1_.ProcessSample();
    float lfo1Mod = lfo1Value * lfo1Amount;

    // Apply LFO1 modulation
    LFO1Target lfo1Target = static_cast<LFO1Target>(lfo1TargetIndex);

    if (lfo1Target == LFO1Target::VCORate) {
        float modulatedVCORate = vcoRate * (1.0f + lfo1Mod * 4.0f);
        vcoRate = juce::jlimit(20.0f, 2000.0f, modulatedVCORate);
    } else if (lfo1Target == LFO1Target::DelayTime) {
        float modulatedDelayTime = delayTime * (1.0f + lfo1Mod * 0.5f);
        delayTime = juce::jlimit(0.001f, 2.0f, modulatedDelayTime);
    } else if (lfo1Target == LFO1Target::DelayFeedback) {
        delayFeedback = juce::jlimit(0.0f, 0.95f, delayFeedback + lfo1Mod * 0.3f);
    }

    // Update DSP modules with modulated values
    dubOscillator_.SetFrequency(vcoRate);
    dubOscillator_.SetLevel(vcoLevel);

    dubDelay_.SetDelayTime(delayTime);
    dubDelay_.SetFeedback(delayFeedback);
    dubDelay_.SetWetDry(delayWetDry);
}

void SimpleSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    // Prepare
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output
    buffer.clear();

    const int numSamples = buffer.getNumSamples();
    float* outputData = buffer.getWritePointer(0);

    // Update static DSP params (LFOs/Delay) before sample loop
    updateDSPFromParameters();

    // Iterate MIDI events with timing and render sample-by-sample so synth only
    // produces audio while a MIDI note is held.
    juce::MidiBuffer::Iterator it(midiMessages);
    juce::MidiMessage msg;
    int samplePos;
    bool hasMsg = it.getNextEvent(msg, samplePos);

    for (int i = 0; i < numSamples; ++i) {
        // process all messages that occur at this sample
        while (hasMsg && samplePos == i) {
            if (msg.isNoteOn()) {
                currentMidiNote_ = msg.getNoteNumber();
                isNoteOn_ = true;
                currentNoteVelocity_ = msg.getFloatVelocity();
                // set oscillator frequency and optionally reset phase
                float freq = SimpleSynth::DSP::MidiNoteToFrequency(currentMidiNote_);
                dubOscillator_.SetFrequency(freq);
                // scale level by velocity
                float baseLevel = parameters_.getRawParameterValue("vcoLevel")->load();
                dubOscillator_.SetLevel(baseLevel * currentNoteVelocity_);
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0)) {
                int note = msg.getNoteNumber();
                if (note == currentMidiNote_)
                {
                    isNoteOn_ = false;
                    currentMidiNote_ = -1;
                }
            }

            hasMsg = it.getNextEvent(msg, samplePos);
        }

        float outSample = 0.0f;
        if (isNoteOn_) {
            outSample = dubOscillator_.ProcessSample();
        }

        outputData[i] = outSample;
    }

    // Apply delay effect to the generated audio (delay will produce tails)
    dubDelay_.Process(outputData, static_cast<size_t>(numSamples));
}

//==============================================================================
bool SimpleSynthProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SimpleSynthProcessor::createEditor()
{
    return new SimpleSynthEditor(*this);
}

//==============================================================================
void SimpleSynthProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SimpleSynthProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters_.state.getType()))
            parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSynthProcessor();
}
