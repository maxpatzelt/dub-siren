#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Custom LookAndFeel for rasta-colored solid knobs
class RastaKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RastaKnobLookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
    
    void setKnobColour(int index);
    
private:
    juce::Colour currentKnobColour;
};

class SimpleSynthEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleSynthEditor(SimpleSynthProcessor&);
    ~SimpleSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SimpleSynthProcessor& processorRef;

    // Controls
    juce::Slider vcoRateSlider, vcoLevelSlider;
    juce::Slider delayTimeSlider, delayFeedbackSlider, delayWetDrySlider;
    juce::Slider lfo1RateSlider, lfo1AmountSlider;
    juce::Slider lfo2RateSlider, lfo2AmountSlider;

    juce::ComboBox lfo1TargetBox, lfo2TargetBox;
    
    // Static labels for knobs
    juce::Label vcoRateLabel, vcoLevelLabel;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayWetDryLabel;
    juce::Label lfo1RateLabel, lfo1AmountLabel;
    juce::Label lfo2RateLabel, lfo2AmountLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> vcoRateAttach, vcoLevelAttach;
    std::unique_ptr<Attachment> delayTimeAttach, delayFeedbackAttach, delayWetDryAttach;
    std::unique_ptr<Attachment> lfo1RateAttach, lfo1AmountAttach;
    std::unique_ptr<Attachment> lfo2RateAttach, lfo2AmountAttach;

    std::unique_ptr<ChoiceAttachment> lfo1TargetAttach, lfo2TargetAttach;

    // Background panel image
    juce::Image panelImage;
    
    // Custom LookAndFeel instances for each knob with different rasta colors
    std::array<std::unique_ptr<RastaKnobLookAndFeel>, 9> knobLookAndFeels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSynthEditor)
};

