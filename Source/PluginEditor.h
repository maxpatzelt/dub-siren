#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/**
 * SimpleSynth Editor (GUI)
 *
 * Minimal editor for now - just displays plugin name.
 * Future phases will add:
 * - Waveform selector
 * - ADSR sliders
 * - Oscilloscope/visualizer
 * - Modulation matrix UI
 */
class SimpleSynthEditor : public juce::AudioProcessorEditor
{
public:
    SimpleSynthEditor(SimpleSynthProcessor&);
    ~SimpleSynthEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SimpleSynthProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthEditor)
};
