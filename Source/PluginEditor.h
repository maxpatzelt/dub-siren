#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

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

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> vcoRateAttach, vcoLevelAttach;
    std::unique_ptr<Attachment> delayTimeAttach, delayFeedbackAttach, delayWetDryAttach;
    std::unique_ptr<Attachment> lfo1RateAttach, lfo1AmountAttach;
    std::unique_ptr<Attachment> lfo2RateAttach, lfo2AmountAttach;

    std::unique_ptr<ChoiceAttachment> lfo1TargetAttach, lfo2TargetAttach;

    // Background panel image
    juce::Image panelImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSynthEditor)
};

