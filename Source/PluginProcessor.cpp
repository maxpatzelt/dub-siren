#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <cmath>
#include <atomic>
#include <BinaryData.h>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
DubSirenProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // 5 macro knobs — all normalised 0..1
    // Everything in the DSP is derived from these at block rate
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "macro1", "PITCH",  juce::NormalisableRange<float>(0.0f, 1.0f), 0.38f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "macro2", "SWEEP",  juce::NormalisableRange<float>(0.0f, 1.0f), 0.45f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "macro3", "WOBBLE", juce::NormalisableRange<float>(0.0f, 1.0f), 0.30f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "macro4", "ECHO",   juce::NormalisableRange<float>(0.0f, 1.0f), 0.50f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "macro5", "CHAOS",  juce::NormalisableRange<float>(0.0f, 1.0f), 0.20f));

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
    slewedVCOFreq_ = 440.0f;

    // Decode all embedded WAV samples (done once at prepare time, not audio thread)
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        const char* data[NUM_SAMPLES] = {
            BinaryData::ds01_wav, BinaryData::ds02_wav, BinaryData::ds03_wav,
            BinaryData::ds04_wav, BinaryData::ds05_wav, BinaryData::ds06_wav,
            BinaryData::ds07_wav, BinaryData::ds08_wav, BinaryData::ds09_wav,
            BinaryData::ds10_wav, BinaryData::ds11_wav, BinaryData::ds12_wav,
            BinaryData::ds13_wav, BinaryData::ds14_wav
        };
        const int sizes[NUM_SAMPLES] = {
            BinaryData::ds01_wavSize, BinaryData::ds02_wavSize, BinaryData::ds03_wavSize,
            BinaryData::ds04_wavSize, BinaryData::ds05_wavSize, BinaryData::ds06_wavSize,
            BinaryData::ds07_wavSize, BinaryData::ds08_wavSize, BinaryData::ds09_wavSize,
            BinaryData::ds10_wavSize, BinaryData::ds11_wavSize, BinaryData::ds12_wavSize,
            BinaryData::ds13_wavSize, BinaryData::ds14_wavSize
        };

        for (int i = 0; i < NUM_SAMPLES; ++i)
        {
            auto* stream = new juce::MemoryInputStream(data[i], (size_t)sizes[i], false);
            std::unique_ptr<juce::AudioFormatReader> reader(
                formatManager.createReaderFor(std::unique_ptr<juce::InputStream>(stream)));

            if (reader != nullptr)
            {
                sampleSourceRates_[i] = reader->sampleRate;
                sampleBuffers_[i].setSize((int)reader->numChannels,
                                          (int)reader->lengthInSamples);
                reader->read(&sampleBuffers_[i], 0, (int)reader->lengthInSamples, 0, true, true);
            }
        }

        // Point player at current selection
        auto& buf = sampleBuffers_[currentSampleIdx_];
        if (buf.getNumSamples() > 0)
            samplePlayer_.Load(buf.getReadPointer(0), (size_t)buf.getNumSamples());
    }
}

void DubSirenProcessor::releaseResources() {}

void DubSirenProcessor::triggerButtonOn()     { buttonNoteOnPending_.store(true);  }
void DubSirenProcessor::triggerButtonOff()    { buttonNoteOffPending_.store(true); }
void DubSirenProcessor::sampleButtonTrigger() { sampleTriggerPending_.store(true); }
void DubSirenProcessor::selectSample(int idx) { sampleSelectPending_.store(idx);   }

//==============================================================================
void DubSirenProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer&         midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const int   numSamples = buffer.getNumSamples();
    const float sampleRate = static_cast<float>(getSampleRate());

    // ── Derive all DSP parameters from 5 macro knobs ─────────────────────────
    const float k1 = *parameters_.getRawParameterValue("macro1"); // PITCH
    const float k2 = *parameters_.getRawParameterValue("macro2"); // SWEEP
    const float k3 = *parameters_.getRawParameterValue("macro3"); // WOBBLE
    const float k4 = *parameters_.getRawParameterValue("macro4"); // ECHO
    const float k5 = *parameters_.getRawParameterValue("macro5"); // CHAOS

    // PITCH (k1): VCO frequency log 30–9000Hz, sample speed, filter brightness
    const float baseVCORate       = 30.0f * std::pow(300.0f, k1);
    const float baseSamplePlayRate = 0.3f + 1.7f * k1;

    // SWEEP (k2): filter cutoff log 200–8000Hz, resonance bell, waveform, level balance
    const float filterSweep      = 200.0f * std::pow(40.0f, k2);
    const float filterResonance  = juce::jlimit(0.0f, 0.98f, 4.0f * k2 * (1.0f - k2) * 1.2f);
    const float filterDecay      = 0.1f + 1.8f * k2;
    const float vcoLevel         = 0.35f + 0.65f * k2;
    const float sampleLevel      = 1.0f - 0.55f * k2;
    const int   vcoWaveformIdx   = (k2 < 0.33f) ? 0 : (k2 < 0.67f) ? 1 : 2; // Sq/Saw/Tri

    // WOBBLE (k3): LFO1 rate/depth log 0.05–12Hz, portamento bell, attenuates LFO2
    const float baseLFO1Rate    = 0.05f * std::pow(240.0f, k3);
    const float lfo1Amount      = k3 * 0.8f;
    const float portamento      = 4.0f * k3 * (1.0f - k3) * 2.0f; // bell max ~2s
    const float lfo2RateK3mod   = 1.0f - 0.55f * k3;

    // ECHO (k4): delay time log 0.05–1.5s, feedback/wet, attenuates LFO2
    const float baseDelayTime     = 0.05f * std::pow(30.0f, k4);
    const float baseDelayFeedback = 0.3f + 0.6f * k4;
    const float baseDelayWetDry   = 0.1f + 0.5f * k4;
    const float lfo2RateK4mod     = 1.0f - 0.4f * k4;

    // CHAOS (k5): LFO2 rate log 0.05–12Hz (inversely modulated by k3+k4),
    //             boosts LFO1, escalates LFO2 target
    const float lfo2Rate     = juce::jlimit(0.05f, 12.0f,
        0.05f * std::pow(240.0f, k5) * lfo2RateK3mod * lfo2RateK4mod);
    const float lfo2Amount   = k5 * 0.9f;
    // CHAOS amplifies LFO1 rate further
    const float lfo1Rate     = juce::jlimit(0.05f, 12.0f, baseLFO1Rate * (1.0f + k5 * 1.5f));
    // LFO2 target escalates: None → LFO1Rate → Delay
    const int   lfo2TargetIdx    = (k5 < 0.33f) ? 0 : (k5 < 0.66f) ? 1 : 3;
    // LFO1 always modulates VCO rate; LFO2 gets S+H waveform at high CHAOS
    const int   lfo1TargetIdx    = 1; // LFO1Target::VCORate
    const int   lfo1WaveformIdx  = 0; // Sine
    const int   lfo2WaveformIdx  = (k5 > 0.7f) ? 3 : 0; // S+H or Sine

    // MIDI always triggers both VCO and sample
    const bool midiToVCO    = true;
    const bool midiToSample = true;

    // ── Apply waveform / rate selections (block-rate, not per-sample) ─────────
    dubOscillator_.SetWaveform(static_cast<DubSiren::DSP::DubOscillator::Waveform>(vcoWaveformIdx));
    dubOscillator_.SetLevel(vcoLevel);

    lfo1_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo1WaveformIdx));
    lfo1_.SetRate(lfo1Rate);

    lfo2_.SetWaveform(static_cast<DubSiren::DSP::LFO::Waveform>(lfo2WaveformIdx));
    lfo2_.SetRate(lfo2Rate);

    // Apply base sample playback rate, compensating for source vs host sample rate
    const float srcRate  = static_cast<float>(sampleSourceRates_[currentSampleIdx_]);
    const float rateComp = (srcRate > 0.0f) ? srcRate / static_cast<float>(getSampleRate()) : 1.0f;
    samplePlayer_.SetPlaybackRate(baseSamplePlayRate * rateComp);

    // Portamento coefficient — 1-pole slew on VCO frequency
    const float portaCoeff = (portamento < 0.001f) ? 0.0f
        : std::exp(-1.0f / (portamento * 2.0f * sampleRate));

    // Filter decay coefficient — cutoff slews toward 200Hz after note-off
    const float filterDecayCoeff = std::exp(-1.0f / (filterDecay * sampleRate));
    const float svfQ = juce::jlimit(0.1f, 2.0f,
        2.0f * (1.0f - filterResonance * 0.92f));

    // Decay cutoff toward 200Hz each block (smooth envelope release)
    if (filterDecaying_)
        currentFilterCutoff_ = filterDecayCoeff * currentFilterCutoff_
                              + (1.0f - filterDecayCoeff) * 200.0f;
    // svfF computed per-sample inside loop (allows LFO modulation)

    // ── Button trigger (UI thread → audio thread via atomics) ────────────────
    if (buttonNoteOnPending_.exchange(false))
    {
        currentMidiNote_      = -1;  // use VCO rate knob, not MIDI pitch
        isNoteOn_             = true;
        currentNoteVelocity_  = 1.0f;
        dubOscillator_.SetLevel(vcoLevel);
        envelope_.NoteOn();
    }
    if (buttonNoteOffPending_.exchange(false))
    {
        isNoteOn_        = false;
        currentMidiNote_ = -1;
        envelope_.NoteOff();
    }

    // Sample selection change (swap buffer pointer at block boundary)
    {
        const int sel = sampleSelectPending_.exchange(-1);
        if (sel >= 0 && sel < NUM_SAMPLES)
        {
            currentSampleIdx_ = sel;
            auto& buf = sampleBuffers_[currentSampleIdx_];
            if (buf.getNumSamples() > 0)
                samplePlayer_.Load(buf.getReadPointer(0), (size_t)buf.getNumSamples());
        }
    }

    if (sampleTriggerPending_.exchange(false))
    {
        // One-shot: always retrigger from the start
        sampleLooping_.store(false);
        samplePlayer_.SetLoop(false);
        samplePlayer_.Trigger();
    }

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
                currentNoteVelocity_ = msg.getFloatVelocity();
                if (midiToVCO)
                {
                    isNoteOn_ = true;
                    dubOscillator_.SetLevel(vcoLevel * currentNoteVelocity_);
                    envelope_.NoteOn();
                    // Snap filter to full SWEEP cutoff on note-on
                    currentFilterCutoff_ = filterSweep;
                    filterDecaying_      = false;
                }
                if (midiToSample)
                {
                    // One-shot: each note-on retriggers sample from start
                    samplePlayer_.SetLoop(false);
                    samplePlayer_.Trigger();
                }
            }
            else if (msg.isNoteOff() || (msg.isNoteOn() && msg.getVelocity() == 0))
            {
                if (msg.getNoteNumber() == currentMidiNote_)
                {
                    if (midiToVCO)
                    {
                        isNoteOn_        = false;
                        currentMidiNote_ = -1;
                        envelope_.NoteOff();
                        filterDecaying_  = true;  // begin filter sweep decay
                    }
                }
            }
            hasMsg = it.getNextEvent(msg, samplePos);
        }

        // ── LFO 2 — tick first so it can modulate LFO1 before LFO1 runs ──
        const float lfo2Value = lfo2_.ProcessSample();
        const float lfo2Mod   = lfo2Value * lfo2Amount;

        // Mutable working values — LFO2 may adjust before LFO1 reads them
        float currentLFO1Rate    = lfo1Rate;
        float currentLFO1Amount  = lfo1Amount;
        float effectiveDelayTime     = baseDelayTime;
        float effectiveDelayFeedback = baseDelayFeedback;
        float effectiveDelayWetDry   = baseDelayWetDry;
        float effectiveSampleRate    = baseSamplePlayRate;
        float effectiveFilterCutoff  = currentFilterCutoff_;

        switch (static_cast<LFO2Target>(lfo2TargetIdx))
        {
            case LFO2Target::LFO1Rate:
                currentLFO1Rate = juce::jlimit(0.05f, 12.0f,
                    lfo1Rate * (1.0f + lfo2Mod * 3.0f));
                lfo1_.SetRate(currentLFO1Rate);
                break;
            case LFO2Target::LFO1Amount:
                // Full depth swing — can drive LFO1 from 0 to 2× its set depth
                currentLFO1Amount = juce::jlimit(0.0f, 1.0f,
                    lfo1Amount * (1.0f + lfo2Mod));
                break;
            case LFO2Target::Delay:
                effectiveDelayTime     = juce::jlimit(0.05f, 2.0f,
                    baseDelayTime * (1.0f + lfo2Mod * 0.25f));
                effectiveDelayFeedback = juce::jlimit(0.0f, 0.99f,
                    baseDelayFeedback + lfo2Mod * 0.15f);
                break;
            case LFO2Target::SampleRate:
                effectiveSampleRate = juce::jlimit(0.05f, 4.0f,
                    baseSamplePlayRate + lfo2Mod * 1.5f);
                break;
            default:
                break;
        }

        // ── LFO 1 — runs after LFO2 so LFO2→LFO1rate/amount is applied ──
        const float lfo1Value = lfo1_.ProcessSample();
        const float lfo1Mod   = lfo1Value * currentLFO1Amount;

        // Base VCO frequency — MIDI note if held, else front-panel SIREN knob
        float targetVCORate = (currentMidiNote_ >= 0)
            ? DubSiren::DSP::MidiNoteToFrequency(currentMidiNote_)
            : baseVCORate;

        switch (static_cast<LFO1Target>(lfo1TargetIdx))
        {
            case LFO1Target::VCORate:
                targetVCORate = juce::jlimit(20.0f, 2000.0f,
                    targetVCORate * (1.0f + lfo1Mod * 4.0f));
                break;
            case LFO1Target::Delay:
                effectiveDelayTime     = juce::jlimit(0.05f, 2.0f,
                    effectiveDelayTime * (1.0f + lfo1Mod * 0.25f));
                effectiveDelayFeedback = juce::jlimit(0.0f, 0.99f,
                    effectiveDelayFeedback + lfo1Mod * 0.15f);
                break;
            case LFO1Target::Filter:
                effectiveFilterCutoff = juce::jlimit(200.0f, 9000.0f,
                    currentFilterCutoff_ * (1.0f + lfo1Mod * 0.8f));
                break;
            case LFO1Target::SampleRate:
                effectiveSampleRate = juce::jlimit(0.05f, 4.0f,
                    effectiveSampleRate + lfo1Mod * 1.5f);
                break;
            default:
                break;
        }

        // ── Apply modulated values ────────────────────────────────────────
        samplePlayer_.SetPlaybackRate(effectiveSampleRate);

        // Portamento slew on VCO frequency
        slewedVCOFreq_ = slewedVCOFreq_ * portaCoeff
                       + targetVCORate  * (1.0f - portaCoeff);
        dubOscillator_.SetFrequency(slewedVCOFreq_);

        dubDelay_.SetDelayTime(effectiveDelayTime);
        dubDelay_.SetFeedback(effectiveDelayFeedback);
        dubDelay_.SetWetDry(effectiveDelayWetDry);

        // ── Oscillator + envelope ─────────────────────────────────────────
        const float envVal = envelope_.ProcessSample();
        float osc = (envVal > 0.0f) ? dubOscillator_.ProcessSample() * envVal : 0.0f;

        // ── Sample player ─────────────────────────────────────────────────
        const float smp = samplePlayer_.ProcessSample() * sampleLevel;

        // ── State-variable LP filter — covers VCO + sample (Chamberlin SVF)
        const float preFilt = osc + smp;
        const float svfF = 2.0f * std::sin(
            juce::MathConstants<float>::pi * effectiveFilterCutoff / sampleRate);
        filterLow_  += svfF * filterBand_;
        const float svfHigh = preFilt - filterLow_ - svfQ * filterBand_;
        filterBand_ += svfF * svfHigh;

        outL[i] = filterLow_;
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
