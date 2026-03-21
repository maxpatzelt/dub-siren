# Dub Siren

<img width="996" height="780" alt="dubsirenpic" src="https://github.com/user-attachments/assets/8838a1b0-ff4a-4028-a777-d80df43014ed" />

A monophonic VST3 dub synthesizer — gritty square wave VCO, two free-running LFOs with flexible routing, and a dub-style feedback delay with analog wobble.

**Author:** Max Patzelt — https://github.com/maxpatzelt/

---

## Features

- **Dub VCO** — Square wave oscillator (`DubOscillator`) with analog drift simulation, MIDI note tracking, and velocity-sensitive amplitude
- **Dual LFOs** — Two independent sine-wave LFOs with configurable routing targets
- **Dub Delay** — Feedback delay (`DubDelay`) with wet/dry mix and subtle wobble instability; up to 2 seconds delay time
- **ADSR Envelope** — Full attack-decay-sustain-release amplitude shaping per note
- **MIDI** — Note-on/off, pitch tracking, velocity scaling

### LFO Routing

| LFO | Targets |
|-----|---------|
| LFO 1 | VCO Rate · Delay Time · Delay Feedback |
| LFO 2 | LFO 1 Rate · LFO 1 Amount · Delay Wet/Dry |

## Project Structure

```
dub-siren/
├── CMakeLists.txt
├── Source/
│   ├── PluginProcessor.cpp/h     — JUCE entry point, parameter tree, MIDI handling
│   ├── PluginEditor.cpp/h        — UI, RastaKnobLookAndFeel
│   └── DSP/
│       ├── Common.h              — Shared types and constants
│       ├── DubOscillator.cpp/h   — Square wave VCO with analog drift
│       ├── LFO.cpp/h             — Free-running sine LFO (-1 to +1)
│       ├── DubDelay.cpp/h        — Dub delay, feedback, wobble
│       ├── Envelope.cpp/h        — ADSR amplitude envelope
│       ├── Oscillator.cpp/h      — Base band-limited oscillator
│       └── Voice.cpp/h           — Per-note voice state
└── Tests/
    ├── test_Oscillator.cpp
    └── test_Envelope.cpp
```

All DSP lives in `namespace DubSiren::DSP`.

## Parameters

| ID | Name | Range | Default |
|----|------|--------|---------|
| `vco_rate` | VCO Rate | 20–2000 Hz | 200 Hz |
| `vco_level` | VCO Level | 0–1 | 0.8 |
| `delay_time` | Delay Time | 0.05–2.0 s | 0.5 s |
| `delay_feedback` | Feedback | 0–0.95 | 0.4 |
| `delay_wet_dry` | Wet/Dry | 0–1 | 0.5 |
| `lfo1_rate` | LFO 1 Rate | 0.1–10 Hz | 1.0 Hz |
| `lfo1_amount` | LFO 1 Amount | 0–1 | 0.3 |
| `lfo2_rate` | LFO 2 Rate | 0.1–5 Hz | 0.5 Hz |
| `lfo2_amount` | LFO 2 Amount | 0–1 | 0.2 |
| `lfo1_target` | LFO 1 Target | None / VCO Rate / Delay Time / Delay FB | None |
| `lfo2_target` | LFO 2 Target | None / LFO1 Rate / LFO1 Amount / Delay W/D | None |

## Building

### Requirements

- JUCE 7+ (git submodule at `libs/JUCE`)
- CMake 3.15+
- C++17 compiler: MSVC 2019+, Clang 10+, or GCC 9+

### Steps

```bash
# Add JUCE submodule
git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
git submodule update --init --recursive

# Configure & build
cmake -B build
cmake --build build --config Release
```

The VST3 will be at: `build/DubSiren_artefacts/Release/VST3/Dub Siren.vst3`

### Install

**Windows:**
```powershell
Copy-Item -Path "build\DubSiren_artefacts\Release\VST3\Dub Siren.vst3" `
  -Destination "$env:CommonProgramFiles\VST3\" -Recurse -Force
```

**macOS:**
```bash
cp -r "build/DubSiren_artefacts/Release/VST3/Dub Siren.vst3" \
  ~/Library/Audio/Plug-Ins/VST3/
```

## Tests

```bash
# Windows
.\build\Tests\Release\DubSiren_Tests.exe
# macOS/Linux
./build/Tests/DubSiren_Tests
```

## Roadmap

- [ ] ADSR sliders exposed in UI
- [ ] Preset save/load
- [ ] Second oscillator / octave spread
- [ ] Portamento / pitch glide
- [ ] macOS Universal Binary (Intel + Apple Silicon)
- [ ] AAX format (Pro Tools)

## License

MIT

JUCE is licensed under GPL v3 or commercial license — see https://juce.com/juce-7-licence/
