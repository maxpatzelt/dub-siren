#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>

//==============================================================================
RastaKnobLookAndFeel::RastaKnobLookAndFeel()
    : currentKnobColour(juce::Colours::red)
{
}

void RastaKnobLookAndFeel::setKnobColour(int index)
{
    // Cycle through rasta colors: red, yellow, green
    const juce::Colour rastaColors[] = {
        juce::Colour(0xffCC0000),  // Red
        juce::Colour(0xffFFD700),  // Gold/Yellow
        juce::Colour(0xff009900)   // Green
    };
    currentKnobColour = rastaColors[index % 3];
}

void RastaKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                            juce::Slider& slider)
{
    auto radius = juce::jmin(width / 2, height / 2) - 8.0f;
    auto centreX = x + width * 0.5f;
    auto centreY = y + height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw solid filled circle for knob body
    g.setColour(currentKnobColour);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    
    // Draw black outline
    g.setColour(juce::Colours::black);
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 3.0f);
    
    // Draw position indicator (white line from center)
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 4.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    g.setColour(juce::Colours::white);
    g.fillPath(p);
}

//==============================================================================
SimpleSynthEditor::SimpleSynthEditor(SimpleSynthProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& params = processorRef.getParameters();
    
    // Create LookAndFeel instances with different rasta colors for each knob
    for (int i = 0; i < 9; ++i)
    {
        knobLookAndFeels[i] = std::make_unique<RastaKnobLookAndFeel>();
        knobLookAndFeels[i]->setKnobColour(i);
    }

    auto makeKnob = [](juce::Slider& s){
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
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
    
    // Apply custom LookAndFeel to each knob with alternating rasta colors
    vcoRateSlider.setLookAndFeel(knobLookAndFeels[0].get());
    vcoLevelSlider.setLookAndFeel(knobLookAndFeels[1].get());
    delayTimeSlider.setLookAndFeel(knobLookAndFeels[2].get());
    delayFeedbackSlider.setLookAndFeel(knobLookAndFeels[3].get());
    delayWetDrySlider.setLookAndFeel(knobLookAndFeels[4].get());
    lfo1RateSlider.setLookAndFeel(knobLookAndFeels[5].get());
    lfo1AmountSlider.setLookAndFeel(knobLookAndFeels[6].get());
    lfo2RateSlider.setLookAndFeel(knobLookAndFeels[7].get());
    lfo2AmountSlider.setLookAndFeel(knobLookAndFeels[8].get());

    addAndMakeVisible(vcoRateSlider);
    addAndMakeVisible(vcoLevelSlider);
    addAndMakeVisible(delayTimeSlider);
    addAndMakeVisible(delayFeedbackSlider);
    addAndMakeVisible(delayWetDrySlider);
    addAndMakeVisible(lfo1RateSlider);
    addAndMakeVisible(lfo1AmountSlider);
    addAndMakeVisible(lfo2RateSlider);
    addAndMakeVisible(lfo2AmountSlider);
    
    // Setup static labels with dub/reggae themed names
    auto setupLabel = [](juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(14.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colours::black.withAlpha(0.8f));
    };
    
    setupLabel(vcoRateLabel, "SIREN");
    setupLabel(vcoLevelLabel, "LEVEL");
    setupLabel(delayTimeLabel, "ECHO");
    setupLabel(delayFeedbackLabel, "REPEATS");
    setupLabel(delayWetDryLabel, "DUB MIX");
    setupLabel(lfo1RateLabel, "WOBBLE");
    setupLabel(lfo1AmountLabel, "DEPTH");
    setupLabel(lfo2RateLabel, "SHAKE");
    setupLabel(lfo2AmountLabel, "POWER");
    
    addAndMakeVisible(vcoRateLabel);
    addAndMakeVisible(vcoLevelLabel);
    addAndMakeVisible(delayTimeLabel);
    addAndMakeVisible(delayFeedbackLabel);
    addAndMakeVisible(delayWetDryLabel);
    addAndMakeVisible(lfo1RateLabel);
    addAndMakeVisible(lfo1AmountLabel);
    addAndMakeVisible(lfo2RateLabel);
    addAndMakeVisible(lfo2AmountLabel);

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

    int knobSize = 135;  // Halfway between 90 and 180

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
    
    // Position labels overlaid on knobs
    auto labelHeight = 20;
    vcoRateLabel.setBounds(50, 80 + knobSize/2 - 10, knobSize, labelHeight);
    vcoLevelLabel.setBounds(240, 80 + knobSize/2 - 10, knobSize, labelHeight);
    delayTimeLabel.setBounds(580, 80 + knobSize/2 - 10, knobSize, labelHeight);
    delayFeedbackLabel.setBounds(120, 240 + knobSize/2 - 10, knobSize, labelHeight);
    delayWetDryLabel.setBounds(520, 240 + knobSize/2 - 10, knobSize, labelHeight);
    lfo1RateLabel.setBounds(80, 400 + knobSize/2 - 10, knobSize, labelHeight);
    lfo1AmountLabel.setBounds(80, 490 + knobSize/2 - 10, knobSize, labelHeight);
    lfo2RateLabel.setBounds(540, 400 + knobSize/2 - 10, knobSize, labelHeight);
    lfo2AmountLabel.setBounds(540, 490 + knobSize/2 - 10, knobSize, labelHeight);
}
