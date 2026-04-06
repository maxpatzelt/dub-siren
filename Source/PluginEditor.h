#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// ── Dub LookAndFeel ───────────────────────────────────────────────────────────
class DubLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DubLookAndFeel();

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isMouseOverButton, bool isButtonDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox&) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

private:
    static constexpr uint32_t AMBER     = 0xFFFFB300;
    static constexpr uint32_t AMBER_DIM = 0xFF3A2800;
    static constexpr uint32_t CREAM     = 0xFFCFC9A8;
    static constexpr uint32_t KNOB_DARK = 0xFF1A1610;
    static constexpr uint32_t KNOB_RIM  = 0xFF2E2618;
};

// ── Editor ───────────────────────────────────────────────────────────────────
class DubSirenEditor : public juce::AudioProcessorEditor
{
public:
    explicit DubSirenEditor(DubSirenProcessor&);
    ~DubSirenEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DubSirenProcessor& processorRef;
    DubLookAndFeel     dubLAF;

    // ── 5 macro knobs ────────────────────────────────────────────────────────
    juce::Slider macroKnob[5];
    juce::Label  macroLabel[5];

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttach> macroAttach[5];

    // ── Bottom button row (one pair per patch slot, then RANDOM + TRIGGER) ─────
    juce::TextButton patchButton[3];       // P1 P2 P3 — recall (larger)
    juce::TextButton patchWriteButton[3];  // W  W  W  — write/save (smaller)
    juce::TextButton randomizeButton;      // RANDOM
    juce::TextButton triggerButton;        // TRIGGER — hold to sustain VCO
    bool             buttonIsDown_ = false;

    // Non-persistent patch memory (lost on plug-in close)
    struct PatchSlot { bool filled = false; float values[5] = {}; };
    PatchSlot patches_[3];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DubSirenEditor)
};
