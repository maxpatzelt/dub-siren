#include "PluginEditor.h"

//==============================================================================
SimpleSynthEditor::SimpleSynthEditor(SimpleSynthProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& params = processorRef.getParameters();

    // Slider styles
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

    // Combo boxes
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

    // Attachments
    vcoRateAttach = std::make_unique<Attachment>(params, "vcoRate", vcoRateSlider);
    vcoLevelAttach = std::make_unique<Attachment>(params, "vcoLevel", vcoLevelSlider);

    delayTimeAttach = std::make_unique<Attachment>(params, "delayTime", delayTimeSlider);
    delayFeedbackAttach = std::make_unique<Attachment>(params, "delayFeedback", delayFeedbackSlider);
    delayWetDryAttach = std::make_unique<Attachment>(params, "delayWetDry", delayWetDrySlider);

    lfo1RateAttach = std::make_unique<Attachment>(params, "lfo1Rate", lfo1RateSlider);
    lfo1AmountAttach = std::make_unique<Attachment>(params, "lfo1Amount", lfo1AmountSlider);
    lfo2RateAttach = std::make_unique<Attachment>(params, "lfo2Rate", lfo2RateSlider);
    lfo2AmountAttach = std::make_unique<Attachment>(params, "lfo2Amount", lfo2AmountSlider);

    lfo1TargetAttach = std::make_unique<ChoiceAttachment>(params, "lfo1Target", lfo1TargetBox);
    lfo2TargetAttach = std::make_unique<ChoiceAttachment>(params, "lfo2Target", lfo2TargetBox);

    setSize(800, 360);
}

SimpleSynthEditor::~SimpleSynthEditor() = default;

void SimpleSynthEditor::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // Rasta gradient background (red -> yellow -> green)
    juce::ColourGradient grad(juce::Colours::red, 0.0f, 0.0f,
                              juce::Colours::yellow, 0.0f, area.getHeight() * 0.5f, false);
    grad.addColour(1.0, juce::Colours::green);
    g.setGradientFill(grad);
    g.fillAll();

    // Stylized graffiti splatter (simple circles)
    for (int i = 0; i < 16; ++i)
    {
        float x = (float) (std::sin(i * 13.0) * 0.5 + 0.5) * area.getWidth();
        float y = (float) (std::cos(i * 7.0) * 0.5 + 0.5) * area.getHeight();
        float r = 8.0f + (i % 5) * 6.0f;
        juce::Colour c = juce::Colour::fromHSV((i * 0.07f), 0.9f, 0.9f, 0.35f);
        g.setColour(c);
        g.fillEllipse(x - r * 0.5f, y - r * 0.5f, r, r);
    }

    // Title
    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.setFont(juce::Font(26.0f, juce::Font::bold));
    g.drawText("Dub Siren", 10, 8, getWidth() - 20, 34, juce::Justification::centred);
}

void SimpleSynthEditor::resized()
{
    auto r = getLocalBounds().reduced(12);

    auto top = r.removeFromTop(48);

    // Layout knobs in two rows
    auto row = r.removeFromTop(140);
    int colWidth = row.getWidth() / 5;

    vcoRateSlider.setBounds(row.removeFromLeft(colWidth).reduced(8));
    vcoLevelSlider.setBounds(row.removeFromLeft(colWidth).reduced(8));
    delayTimeSlider.setBounds(row.removeFromLeft(colWidth).reduced(8));
    delayFeedbackSlider.setBounds(row.removeFromLeft(colWidth).reduced(8));
    delayWetDrySlider.setBounds(row.removeFromLeft(colWidth).reduced(8));

    auto row2 = r.removeFromTop(140);
    int col2 = row2.getWidth() / 5;

    lfo1RateSlider.setBounds(row2.removeFromLeft(col2).reduced(8));
    lfo1AmountSlider.setBounds(row2.removeFromLeft(col2).reduced(8));
    lfo1TargetBox.setBounds(row2.removeFromLeft(col2).reduced(24));
    lfo2RateSlider.setBounds(row2.removeFromLeft(col2).reduced(8));
    lfo2AmountSlider.setBounds(row2.removeFromLeft(col2).reduced(8));

    // place LFO2 target under the last knob area
    lfo2TargetBox.setBounds(getWidth() - col2 - 24, row2.getY() + row2.getHeight() - 40, col2 - 16, 32);
}
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
