#include "PluginEditor.h"
#include <BinaryData.h>

// ============================================================================
// DubLookAndFeel
// ============================================================================

DubLookAndFeel::DubLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId,    juce::Colour(AMBER));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(AMBER_DIM));
    setColour(juce::Label::textColourId,                 juce::Colour(CREAM));
    setColour(juce::ComboBox::backgroundColourId,        juce::Colour(0x88141210));
    setColour(juce::ComboBox::textColourId,              juce::Colour(AMBER));
    setColour(juce::ComboBox::outlineColourId,           juce::Colour(0xFF3A3020));
    setColour(juce::ComboBox::arrowColourId,             juce::Colour(AMBER));
    setColour(juce::PopupMenu::backgroundColourId,       juce::Colour(0xFF1A1710));
    setColour(juce::PopupMenu::textColourId,             juce::Colour(CREAM));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF3A2800));
    setColour(juce::PopupMenu::highlightedTextColourId,  juce::Colour(AMBER));
}

void DubLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                      int x, int y, int width, int height,
                                      float sliderPos,
                                      float startAngle, float endAngle,
                                      juce::Slider& /*slider*/)
{
    const float cx     = x + width  * 0.5f;
    const float cy     = y + height * 0.5f;
    const float radius = juce::jmin(width, height) * 0.5f - 5.0f;
    const float angle  = startAngle + sliderPos * (endAngle - startAngle);

    const float trackW = 3.5f;
    {
        juce::Path track;
        track.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, endAngle, true);
        g.setColour(juce::Colour(AMBER_DIM));
        g.strokePath(track, juce::PathStrokeType(trackW, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }
    {
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(juce::Colour(AMBER));
        g.strokePath(arc, juce::PathStrokeType(trackW, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }

    const float kr = radius - trackW - 2.0f;
    {
        juce::ColourGradient bodyGrad(
            juce::Colour(0xFF3A3020), cx - kr * 0.35f, cy - kr * 0.35f,
            juce::Colour(KNOB_DARK),  cx + kr * 0.35f, cy + kr * 0.35f,
            true);
        bodyGrad.addColour(0.5, juce::Colour(0xFF2A2418));
        g.setGradientFill(bodyGrad);
        g.fillEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f);
    }

    g.setColour(juce::Colour(KNOB_RIM));
    g.drawEllipse(cx - kr, cy - kr, kr * 2.0f, kr * 2.0f, 1.2f);
    g.setColour(juce::Colour(0x22FFFFFF));
    g.drawEllipse(cx - kr + 1.5f, cy - kr + 1.5f, kr * 0.9f, kr * 0.9f, 0.8f);

    {
        const float lineStart = kr * 0.25f;
        const float lineEnd   = kr * 0.85f;
        const float sinA      = std::sin(angle);
        const float cosA      = std::cos(angle);
        g.setColour(juce::Colour(CREAM).withAlpha(0.9f));
        g.drawLine(cx + sinA * lineStart, cy - cosA * lineStart,
                   cx + sinA * lineEnd,   cy - cosA * lineEnd,   1.8f);
        g.setColour(juce::Colour(AMBER).withAlpha(0.8f));
        g.fillEllipse(cx + sinA * (lineEnd - 1.0f) - 2.0f,
                      cy - cosA * (lineEnd - 1.0f) - 2.0f, 4.0f, 4.0f);
    }

    const float screwR = kr * 0.18f;
    g.setColour(juce::Colour(0xFF181410));
    g.fillEllipse(cx - screwR, cy - screwR, screwR * 2.0f, screwR * 2.0f);
    g.setColour(juce::Colour(0xFF403820));
    g.drawEllipse(cx - screwR, cy - screwR, screwR * 2.0f, screwR * 2.0f, 0.8f);
    const float slotAngle = angle;
    g.setColour(juce::Colour(0xFF504428));
    g.drawLine(cx + std::sin(slotAngle) * (-screwR * 0.7f),
               cy - std::cos(slotAngle) * (-screwR * 0.7f),
               cx + std::sin(slotAngle) * ( screwR * 0.7f),
               cy - std::cos(slotAngle) * ( screwR * 0.7f), 1.0f);
}

void DubLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                  bool /*isButtonDown*/,
                                  int /*buttonX*/, int /*buttonY*/,
                                  int /*buttonW*/, int /*buttonH*/,
                                  juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
    g.setColour(juce::Colour(0x88141210));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colour(0xAA4A3A18));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

    const float arrowX = width - 14.0f;
    const float arrowY = height * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX,        arrowY - 3.0f);
    arrow.lineTo         (arrowX + 6.0f, arrowY - 3.0f);
    arrow.lineTo         (arrowX + 3.0f, arrowY + 3.0f);
    arrow.closeSubPath();
    g.setColour(juce::Colour(AMBER).withAlpha(0.7f));
    g.fillPath(arrow);
}

void DubLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType(), false);
}

void DubLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                          const juce::Colour&,
                                          bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    juce::ColourGradient body(
        juce::Colour(isButtonDown ? 0xFF4A3200 : 0xFF1E1810), bounds.getX(), bounds.getY(),
        juce::Colour(isButtonDown ? 0xFF261800 : 0xFF0E0C08), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(body);
    g.fillRoundedRectangle(bounds, 3.0f);

    const auto borderColour = isButtonDown    ? juce::Colour(0xFFFFB300)
                            : isMouseOverButton ? juce::Colour(0xFF705030)
                                               : juce::Colour(0xFF3A2800);
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.2f);

    if (isButtonDown)
    {
        g.setColour(juce::Colour(0x28FFB300));
        g.fillRoundedRectangle(bounds.expanded(3.0f), 6.0f);
    }
}

void DubLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                    bool /*isMouseOver*/, bool isButtonDown)
{
    g.setColour(juce::Colour(isButtonDown ? 0xFFFFB300 : 0xFF806040));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), button.getLocalBounds(),
               juce::Justification::centred, false);
}

// ============================================================================
// Helpers
// ============================================================================
static void setupKnob(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
}

static void setupLabel(juce::Label& lbl, const juce::String& text)
{
    lbl.setText(text, juce::dontSendNotification);
    lbl.setJustificationType(juce::Justification::centred);
    lbl.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
    lbl.setColour(juce::Label::textColourId, juce::Colour(0xFFCFC9A8).withAlpha(0.8f));
}

// ============================================================================
// DubSirenEditor — constructor
// ============================================================================
DubSirenEditor::DubSirenEditor(DubSirenProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&dubLAF);

    // Sample selector and FIRE button removed — sample is randomised by RANDOM button,
    // and TRIGGER below fires both VCO and sample.

    // ── 5 macro knobs ────────────────────────────────────────────────────────
    const char* macroNames[] = { "PITCH", "SWEEP", "WOBBLE", "ECHO", "CHAOS" };
    const char* macroIds[]   = { "macro1", "macro2", "macro3", "macro4", "macro5" };
    auto& params = processorRef.getParameters();

    for (int i = 0; i < 5; ++i)
    {
        setupKnob(macroKnob[i]);
        addAndMakeVisible(macroKnob[i]);

        setupLabel(macroLabel[i], macroNames[i]);
        addAndMakeVisible(macroLabel[i]);

        macroAttach[i] = std::make_unique<SliderAttach>(params, macroIds[i], macroKnob[i]);
    }

    // ── Patch recall + write buttons (P1/P2/P3 recall, W write) ─────────────
    for (int i = 0; i < 3; ++i)
    {
        // Recall button — larger, left side
        patchButton[i].setButtonText("P" + juce::String(i + 1));
        const int slot = i;
        patchButton[i].onClick = [this, slot, macroIds]()
        {
            auto& patch = patches_[slot];
            if (!patch.filled) return;
            for (int k = 0; k < 5; ++k)
                if (auto* param = processorRef.getParameters().getParameter(macroIds[k]))
                    param->setValueNotifyingHost(patch.values[k]);
        };
        addAndMakeVisible(patchButton[i]);

        // Write button — smaller, right side
        patchWriteButton[i].setButtonText("W");
        patchWriteButton[i].onClick = [this, slot, macroIds]()
        {
            auto& patch = patches_[slot];
            for (int k = 0; k < 5; ++k)
                patch.values[k] = *processorRef.getParameters().getRawParameterValue(macroIds[k]);
            patch.filled = true;
            patchButton[slot].setButtonText("P" + juce::String(slot + 1) + "*");
        };
        addAndMakeVisible(patchWriteButton[i]);
    }

    // ── Randomize button ─────────────────────────────────────────────────────
    randomizeButton.setButtonText("RANDOM");
    randomizeButton.onClick = [this]()
    {
        juce::Random rng;
        for (auto* param : static_cast<juce::AudioProcessor&>(processorRef).getParameters())
            param->setValueNotifyingHost(rng.nextFloat());
        const int randomSample = rng.nextInt(processorRef.getNumSamples());
        processorRef.selectSample(randomSample);
    };
    addAndMakeVisible(randomizeButton);

    // ── Trigger button — hold to sustain VCO ─────────────────────────────────
    triggerButton.setButtonText("TRIGGER");
    triggerButton.onStateChange = [this]()
    {
        const bool down = (triggerButton.getState() == juce::Button::buttonDown);
        if (down && !buttonIsDown_)
        {
            processorRef.triggerButtonOn();
            processorRef.sampleButtonTrigger();  // also one-shot the sample
            buttonIsDown_ = true;
        }
        else if (!down && buttonIsDown_)
        {
            processorRef.triggerButtonOff();
            buttonIsDown_ = false;
        }
    };
    addAndMakeVisible(triggerButton);

    setSize(720, 230);
}

DubSirenEditor::~DubSirenEditor()
{
    setLookAndFeel(nullptr);
}

// ============================================================================
// paint
// ============================================================================
void DubSirenEditor::paint(juce::Graphics& g)
{
    const int W = getWidth();
    const int H = getHeight();

    // Background image
    {
        auto img = juce::ImageCache::getFromMemory(
            BinaryData::background_jfif, BinaryData::background_jfifSize);
        if (img.isValid())
        {
            g.drawImage(img, 0, 0, W, H, 0, 0, img.getWidth(), img.getHeight());
            g.setColour(juce::Colour(0x550E0C0A));  // lighter overlay — background shows through
            g.fillAll();
        }
        else
        {
            juce::ColourGradient bg(
                juce::Colour(0xFF161210), 0.0f,    0.0f,
                juce::Colour(0xFF0E0C0A), (float)W, (float)H, false);
            g.setGradientFill(bg);
            g.fillAll();
        }
    }

    // Scanline texture
    g.setColour(juce::Colour(0x07000000));
    for (int y = 0; y < H; y += 2)
        g.drawHorizontalLine(y, 0.0f, (float)W);

    // Header bar
    {
        juce::ColourGradient hdr(
            juce::Colour(0xFF1A1510), 0.0f,  0.0f,
            juce::Colour(0xFF0E0C08), 0.0f, 40.0f, false);
        g.setGradientFill(hdr);
        g.fillRect(0, 0, W, 40);
        g.setColour(juce::Colour(0xFF2E2A1A));
        g.drawHorizontalLine(40, 0.0f, (float)W);
    }

    g.setColour(juce::Colour(0xFFFFB300));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));
    g.drawText("DUB  SIREN", 20, 0, 220, 40, juce::Justification::centredLeft, false);

    g.setColour(juce::Colour(0xFFCFC9A8).withAlpha(0.4f));
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
    g.drawText("AudioRF  v3.0", W - 130, 0, 120, 40, juce::Justification::centredRight, false);

    // Power LED
    const float ledX = W - 18.0f;
    const float ledY = 14.0f;
    g.setColour(juce::Colour(0xFF39FF14).withAlpha(0.8f));
    g.fillEllipse(ledX, ledY, 8.0f, 8.0f);
    g.setColour(juce::Colour(0xFF39FF14).withAlpha(0.15f));
    g.fillEllipse(ledX - 3.0f, ledY - 3.0f, 14.0f, 14.0f);

    // Per-knob column boxes
    const int colW = getWidth() / 5;
    for (int i = 0; i < 5; ++i)
    {
        juce::Rectangle<float> box((float)(i * colW + 4), 43.0f,
                                   (float)(colW - 8), 145.0f);
        g.setColour(juce::Colour(0x0A1E1B16));   // near-invisible fill
        g.fillRoundedRectangle(box, 5.0f);
        g.setColour(juce::Colour(0xBB4A3A18));   // amber border unchanged
        g.drawRoundedRectangle(box.reduced(0.5f), 5.0f, 1.2f);
    }

    // Corner screws
    const int sp[][2] = { {8,8},{W-14,8},{8,H-14},{W-14,H-14} };
    for (auto& s : sp)
    {
        g.setColour(juce::Colour(0xFF2A2418));
        g.fillEllipse((float)s[0], (float)s[1], 6.0f, 6.0f);
        g.setColour(juce::Colour(0xFF504030));
        g.drawEllipse((float)s[0], (float)s[1], 6.0f, 6.0f, 0.8f);
        g.drawLine(s[0]+3.0f, s[1]+1.0f, s[0]+3.0f, s[1]+5.0f, 0.8f);
    }
}

// ============================================================================
// resized
// ============================================================================
void DubSirenEditor::resized()
{
    const int W    = getWidth();
    const int colW = W / 5;

    // Header is title-only — no controls needed

    // ── Knob + label per column ───────────────────────────────────────────────
    const int knobSize  = 110;
    const int knobY     = 47;
    const int labelH    = 14;
    const int labelY    = knobY + knobSize + 4;

    for (int i = 0; i < 5; ++i)
    {
        const int cx = i * colW + (colW - knobSize) / 2;
        macroKnob[i] .setBounds(cx,  knobY, knobSize, knobSize);
        macroLabel[i].setBounds(cx, labelY, knobSize, labelH);
    }

    // ── Bottom button row ─────────────────────────────────────────────────────
    const int btnY    = labelY + labelH + 6;
    const int btnH    = 26;
    const int margin  = 7;
    const int usable  = colW - margin * 2;   // ~130px per column
    const int writeW  = 28;                  // small write button
    const int gap     = 3;
    const int recallW = usable - writeW - gap;  // larger recall button

    for (int i = 0; i < 3; ++i)
    {
        const int x0 = i * colW + margin;
        patchButton[i]      .setBounds(x0,                  btnY, recallW, btnH);
        patchWriteButton[i] .setBounds(x0 + recallW + gap,  btnY, writeW,  btnH);
    }

    randomizeButton.setBounds(3 * colW + margin, btnY, usable, btnH);
    triggerButton  .setBounds(4 * colW + margin, btnY, usable, btnH);
}
