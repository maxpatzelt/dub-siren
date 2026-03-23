#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DubSirenProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ── VCO ────────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoRate", "VCO Rate",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 0.1f, 0.3f), 440.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "vcoLevel", "VCO Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "vcoWaveform", "VCO Waveform",
        juce::StringArray{ "Square", "Saw", "Triangle" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "portamento", "Portamento",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // ── Delay ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f), 0.375f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f), 0.6f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayWetDry", "Delay Wet/Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));

    // ── LFO 1 ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Rate", "LFO 1 Rate",
        juce::NormalisableRange<float>(0.1f, 80.0f, 0.01f, 0.3f), 2.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo1Amount", "LFO 1 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo1Target", "LFO 1 Target",
        juce::StringArray{ "None", "VCO Rate", "Delay Time", "Delay Feedback" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo1Waveform", "LFO 1 Waveform",
        juce::StringArray{ "Sine", "Tri", "Square", "S+H" }, 0));

    // ── LFO 2 ──────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Rate", "LFO 2 Rate",
        juce::NormalisableRange<float>(0.1f, 80.0f, 0.01f, 0.3f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lfo2Amount", "LFO 2 Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo2Target", "LFO 2 Target",
        juce::StringArray{ "None", "LFO1 Rate", "LFO1 Amount", "Delay Wet/Dry" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "lfo2Waveform", "LFO 2 Waveform",
        juce::StringArray{ "Sine", "Tri", "Square", "S+H" }, 0));

    return layout;
}

//==============================================================================
DubSirenProcessor::DubSirenProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters_(*this, nullptr, "DubSiren", createParameterLayout())
{}

DubSirenProcessor::~DubSirenProcessor() {}

//==============================================================================
const juce::String DubSirenProcessor::getName() const { return JucePlugin_Name; }
bool DubSirenProcessor::acceptsMidi()  const { return true;  }
bool DubSirenProcessor::producesMidi() const { return false; }
bool DubSirenProcessor::isMidiEffect() const { return false; }
double DubSirenProcessor::getTailLengthSeconds() const { return 2.0; }

int  DubSirenProcessor::getNumPrograms()            { return 1; }
int  DubSirenProcessor::getCurrentProgram()         { return 0; }
void DubSirenProcessor::setCurrentProgram(int)      {}
const juce::String DubSirenProcessor::getProgramName(int) { return {}; }
void DubSirenProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
bool DubSirenProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto& outs = layouts.getMainOutputChannelSet();
    return outs == juce::AudioChannelSet::mono()
        || outs == juce::AudioChannelSet::stereo();
}

//==============================================================================
void DubSirenProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    dubOscillator_.Init (static_cast<float>(sampleRate));
    lfo1_.Init          (static_cast<float>(sampleRate));
    lfo2_.Init          (static_cast<float>(sampleRate));
    dubDelay_.Init      (static_cast<float>(sampleRate), 2.0f);
    envelope_.Init      (static_cast<float>(sampleRate));

    // Seed the slewedVCOFreq so portamento doesn't slide from 0 on first note
    slewedVCOFreq_ = *parameters_.getRawParameterValue("vcoRate");
}

void DubSirenProcessor::releaseResources() {}

//==============================================================================
void DubSirenProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer&         midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const int   numSamples = buffer.getNumSamples();
    const float sampleRate = static_cast<float>(getSampleRate());

    // ── Cache ALL parameter values at block boundary (never read per-sample) ──
    const float baseVCORate      = *parameters_.getRawParameterValue("vcoRate");
    const float vcoLevel         = *parameters_.getRawParameterValue("vcoLevel");
    const int   vcoWaveformIdx   = static_cast<int>(*parameters_.getRawParameterValue("vcoWaveform"));
    const float portamento       = *parameters_.getRawParameterValue("portamento");

    const float baseDelayTime    = *parameters_.getRawParameterValue("delayTime");
    const float baseDelayFeedback= *parameters_.getRawParameterValue("delayFeedback");
    const float baseDelayWetDry  = *parameters_.getRawParameterValue("delayWetDry");

    const float baseLFO1Rate     = *parameters_.getRawParameterValue("lfo1Rate");
    const float lfo1Amount       = *parameters_.getRawParameterValue("lfo1Amount");
    const int   lfo1TargetIdx    = static_cast<int>(*parameters_.getRawParameterValue("lfo1Target"));
    const int   lfo1WaveformIdx  = static_cast<int>(*parameters_.getRawParameterValue("lfo1Waveform"));

    const float baseLFO2Rate     = *parameters_.getRawParameterValue("lfo2Rate");
    const float lfo2Amount       = *parameters_.getRawParameterValue("lfo2Amount");
    const int   lfo2TargetIdx    = static_cast<int>(*parameters_.getRawParameterValue("lfo2Target"));
    const int   lfo2WaveformIdx  = static_cast<int>(*parameters_.getRawParameterValue("lfo2Waveform"));

    // ── Apply waveform / rate selections (block-rate, not per-sample) ────────
    dubOscillator_.SetWaveform(static_cast<DubSiren::DSP::DubOscillator::Waveform>(vcoWaveformIdx));
    dubOscillator_.SetLevel(vcoLevel);

    lfo1_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo1WaveformIdx));
    lfo1_.SetRate(baseLFO1Rate);

    lfo2_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo2WaveformIdx));
    lfo2_.SetRate(baseLFO2Rate);

    // Portamento coefficient — 1-pole slew on VCO frequency
    // portamento=0 → instant; portamento=1 → ~2 s slew
    const float portaCoeff = (portamento < 0.001f) ? 0.0f
        : std::exp(-1.0f / (portamento * 2.0f * sampleRate));

    // ── MIDI iterator ────────────────────────────────────────────────────────
    juce::MidiBuffer::Iterator it(midiMessages);
    juce::MidiMessage msg;
    int samplePos = 0;
    bool hasMsg   = it.getNextEvent(msg, samplePos);

    float* outL = buffer.getWritePointer(0);
    const bool isStereo = (getTotalNumOutputChannels() > 1);

    // ── Sample loop ──────────────────────────────────────────────────────────
    for (int i = 0; i < numSamples; ++i)
    {
        // Process MIDI events at this sample position
        while (hasMsg && samplePos == i)
        {
            if (msg.isNoteOn())
            {
                currentMidiNote_     = msg.getNoteNumber();
                isNoteOn_            = true;
                currentNoteVelocity_ = msg.getFloatVelocity();
                dubOscillator_.SetLevel(vcoLevel * currentNoteVelocity_);
                envelope_.NoteOn();
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            {
                if (msg.getNoteNumber() == currentMidiNote_)
                {
                    isNoteOn_        = false;
                    currentMidiNote_ = -1;
                    envelope_.NoteOff();
                }
            }
            hasMsg = it.getNextEvent(msg, samplePos);
        }

        // ── LFO 2 — tick first (modulates LFO1 or delay) ─────────────────
        const float lfo2Value = lfo2_.ProcessSample();
        const float lfo2Mod   = lfo2Value * lfo2Amount;

        float currentLFO1Rate   = baseLFO1Rate;
        float currentLFO1Amount = lfo1Amount;
        float currentDelayWetDry= baseDelayWetDry;

        switch (static_cast<LFO2Target>(lfo2TargetIdx))
        {
            case LFO2Target::LFO1Rate:
                currentLFO1Rate = juce::jlimit(0.1f, 80.0f,
                    baseLFO1Rate * (1.0f + lfo2Mod * 3.0f));
                lfo1_.SetRate(currentLFO1Rate);
                break;
            case LFO2Target::LFO1Amount:
                currentLFO1Amount = juce::jlimit(0.0f, 1.0f,
                    lfo1Amount + lfo2Mod * 0.5f);
                break;
            case LFO2Target::DelayWetDry:
                currentDelayWetDry = juce::jlimit(0.0f, 1.0f,
                    baseDelayWetDry + lfo2Mod * 0.3f);
                break;
            default:
                break;
        }

        // ── LFO 1 — tick after LFO2 so LFO2→LFO1rate is applied ─────────
        const float lfo1Value = lfo1_.ProcessSample();
        const float lfo1Mod   = lfo1Value * currentLFO1Amount;

        // Effective VCO target rate for this sample (from MIDI or base param)
        float targetVCORate = (currentMidiNote_ >= 0)
            ? DubSiren::DSP::MidiNoteToFrequency(currentMidiNote_)
            : baseVCORate;

        float effectiveDelayTime     = baseDelayTime;
        float effectiveDelayFeedback = baseDelayFeedback;

        switch (static_cast<LFO1Target>(lfo1TargetIdx))
        {
            case LFO1Target::VCORate:
                targetVCORate = juce::jlimit(20.0f, 2000.0f,
                    targetVCORate * (1.0f + lfo1Mod * 4.0f));
                break;
            case LFO1Target::DelayTime:
                effectiveDelayTime = juce::jlimit(0.001f, 2.0f,
                    baseDelayTime * (1.0f + lfo1Mod * 0.5f));
                break;
            case LFO1Target::DelayFeedback:
                effectiveDelayFeedback = juce::jlimit(0.0f, 0.95f,
                    baseDelayFeedback + lfo1Mod * 0.3f);
                break;
            default:
                break;
        }

        // ── Portamento slew on VCO frequency ─────────────────────────────
        slewedVCOFreq_ = slewedVCOFreq_ * portaCoeff
                       + targetVCORate  * (1.0f - portaCoeff);
        dubOscillator_.SetFrequency(slewedVCOFreq_);

        // Apply delay settings for this sample
        dubDelay_.SetDelayTime(effectiveDelayTime);
        dubDelay_.SetFeedback(effectiveDelayFeedback);
        dubDelay_.SetWetDry(currentDelayWetDry);

        // ── Oscillator + envelope ─────────────────────────────────────────
        const float envVal = envelope_.ProcessSample();
        const float osc    = (envVal > 0.0f) ? dubOscillator_.ProcessSample() * envVal
                                              : 0.0f;

        outL[i] = osc;
    }

    // ── Apply delay to the mono mix ───────────────────────────────────────────
    dubDelay_.Process(outL, static_cast<size_t>(numSamples));

    // ── Copy to right channel for stereo output ───────────────────────────────
    if (isStereo)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
}

//==============================================================================
bool DubSirenProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* DubSirenProcessor::createEditor()
{
    return new DubSirenEditor(*this);
}

//==============================================================================
void DubSirenProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DubSirenProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters_.state.getType()))
        parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DubSirenProcessor();
}
