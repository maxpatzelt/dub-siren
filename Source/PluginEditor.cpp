#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSynthEditor::SimpleSynthEditor(SimpleSynthProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set editor size
    setSize(400, 300);
}

SimpleSynthEditor::~SimpleSynthEditor()
{
}

//==============================================================================
void SimpleSynthEditor::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("SimpleSynth", getLocalBounds(), juce::Justification::centred, 1);

    // Draw subtitle
    g.setFont(14.0f);
    auto textBounds = getLocalBounds().reduced(20);
    textBounds.setY(getHeight() / 2 + 20);
    textBounds.setHeight(100);
    g.drawFittedText("Monophonic Synthesizer\nSaw Wave + ADSR Envelope\n\nPlay MIDI notes to hear sound!",
                     textBounds,
                     juce::Justification::centred,
                     4);
}

void SimpleSynthEditor::resized()
{
    // Layout components here when we add them
}
