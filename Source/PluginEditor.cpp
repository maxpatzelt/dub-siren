#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>

//==============================================================================
SimpleSynthEditor::SimpleSynthEditor(SimpleSynthProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& params = processorRef.getParameters();

    auto makeKnob = [](juce::Slider& s){
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    };

    makeKnob(vcoRateSlider);
    makeKnob(vcoLevelSlider);
    makeKnob(delayTimeSlider);
    makeKnob(delayFeedbackSlider);
    makeKnob(delayWetDrySlider);
    makeKnob(lfo1RateSlider);
    makeKnob(lfo1AmountSlider);
    makeKnob(lfo2RateSlider);
    makeKnob(lfo2AmountSlider);

    addAndMakeVisible(vcoRateSlider);
    addAndMakeVisible(vcoLevelSlider);
    addAndMakeVisible(delayTimeSlider);
    addAndMakeVisible(delayFeedbackSlider);
    addAndMakeVisible(delayWetDrySlider);
    addAndMakeVisible(lfo1RateSlider);
    addAndMakeVisible(lfo1AmountSlider);
    addAndMakeVisible(lfo2RateSlider);
    addAndMakeVisible(lfo2AmountSlider);

    lfo1TargetBox.addItem("None", 1);
    lfo1TargetBox.addItem("VCO Rate", 2);
    lfo1TargetBox.addItem("Delay Time", 3);
    lfo1TargetBox.addItem("Delay Feedback", 4);
    addAndMakeVisible(lfo1TargetBox);

    lfo2TargetBox.addItem("None", 1);
    lfo2TargetBox.addItem("LFO1 Rate", 2);
    lfo2TargetBox.addItem("LFO1 Amount", 3);
    lfo2TargetBox.addItem("Delay Wet/Dry", 4);
    addAndMakeVisible(lfo2TargetBox);

    vcoRateAttach = std::make_unique<Attachment>(processorRef.getParameters(), "vcoRate", vcoRateSlider);
    vcoLevelAttach = std::make_unique<Attachment>(processorRef.getParameters(), "vcoLevel", vcoLevelSlider);

    delayTimeAttach = std::make_unique<Attachment>(processorRef.getParameters(), "delayTime", delayTimeSlider);
    delayFeedbackAttach = std::make_unique<Attachment>(processorRef.getParameters(), "delayFeedback", delayFeedbackSlider);
    delayWetDryAttach = std::make_unique<Attachment>(processorRef.getParameters(), "delayWetDry", delayWetDrySlider);

    lfo1RateAttach = std::make_unique<Attachment>(processorRef.getParameters(), "lfo1Rate", lfo1RateSlider);
    lfo1AmountAttach = std::make_unique<Attachment>(processorRef.getParameters(), "lfo1Amount", lfo1AmountSlider);
    lfo2RateAttach = std::make_unique<Attachment>(processorRef.getParameters(), "lfo2Rate", lfo2RateSlider);
    lfo2AmountAttach = std::make_unique<Attachment>(processorRef.getParameters(), "lfo2Amount", lfo2AmountSlider);

    lfo1TargetAttach = std::make_unique<ChoiceAttachment>(processorRef.getParameters(), "lfo1Target", lfo1TargetBox);
    lfo2TargetAttach = std::make_unique<ChoiceAttachment>(processorRef.getParameters(), "lfo2Target", lfo2TargetBox);

    // Load the panel background image
    panelImage = juce::ImageCache::getFromMemory(BinaryData::panel_jpg, BinaryData::panel_jpgSize);

    setSize(800, 600);
}

SimpleSynthEditor::~SimpleSynthEditor() = default;

void SimpleSynthEditor::paint(juce::Graphics& g)
{
    // Draw the panel background image
    if (panelImage.isValid())
    {
        g.drawImage(panelImage, getLocalBounds().toFloat(),
                    juce::RectanglePlacement::fillDestination);
    }
    else
    {
        // Bright magenta background to show image didn't load
        g.fillAll(juce::Colours::magenta);
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("PANEL IMAGE NOT LOADED", getLocalBounds(), juce::Justification::centred);
    }
}

void SimpleSynthEditor::resized()
{
    auto bounds = getLocalBounds();
    int w = bounds.getWidth();
    int h = bounds.getHeight();

    // Map digital knobs to physical knob positions in panel image
    // Panel has knobs arranged in rows - adjusting for 800x600 window

    int knobSize = 90;

    // Top row - 3 knobs (left, center-left, right)
    vcoRateSlider.setBounds(50, 80, knobSize, knobSize);           // Top left
    vcoLevelSlider.setBounds(240, 80, knobSize, knobSize);         // Top center-left
    delayTimeSlider.setBounds(580, 80, knobSize, knobSize);        // Top right

    // Middle row - 2 knobs
    delayFeedbackSlider.setBounds(120, 240, knobSize, knobSize);   // Middle left
    delayWetDrySlider.setBounds(520, 240, knobSize, knobSize);     // Middle right

    // Bottom row - left side 2 knobs
    lfo1RateSlider.setBounds(80, 400, knobSize, knobSize);         // Bottom left-top
    lfo1AmountSlider.setBounds(80, 490, knobSize, knobSize);       // Bottom left-bottom

    // Bottom row - right side 2 knobs + 1 box
    lfo2RateSlider.setBounds(540, 400, knobSize, knobSize);        // Bottom right-top
    lfo2AmountSlider.setBounds(540, 490, knobSize, knobSize);      // Bottom right-bottom

    // ComboBoxes in available space
    lfo1TargetBox.setBounds(280, 420, 180, 30);                    // Center area
    lfo2TargetBox.setBounds(280, 480, 180, 30);                    // Center area
}
