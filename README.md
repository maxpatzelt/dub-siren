# Dub Siren (SimpleSynth fork) — JUCE VST3 Synthesizer

This branch transforms the original SimpleSynth into "Dub Siren": a mono, dub-inspired VST3 with a gritty VCO, two free-running LFOs, and a wobbling dub delay. The repository still contains the original SimpleSynth components; the plugin code and DSP have been extended to implement the Dub Siren feature set.

Overview: A monophonic, creative sound-design instrument built with JUCE and modern C++ (C++17). Intended for live performance and dub-style modulation.

## Features

- **Band-Limited Oscillator**: Sine, sawtooth, and square waveforms with polyBLEP anti-aliasing
- **ADSR Envelope**: Full attack-decay-sustain-release amplitude shaping
- **Monophonic Voice**: Single voice architecture ready for future polyphony
- **Automated Tests**: Comprehensive unit tests for all DSP components
- **Clean Architecture**: Modular DSP core separate from JUCE plugin framework

## Project Structure

```
SimpleSynth/
├── CMakeLists.txt              # Root build configuration
├── Source/
│   ├── PluginProcessor.cpp/h   # Main plugin interface
│   ├── PluginEditor.cpp/h      # GUI (minimal for now)
│   └── DSP/
│       ├── Common.h            # Shared utilities and constants
│       ├── Oscillator.cpp/h    # Band-limited waveform generator
│       ├── Envelope.cpp/h      # ADSR envelope generator
│       └── Voice.cpp/h         # Complete synth voice
└── Tests/
    ├── CMakeLists.txt          # Test build configuration
    ├── test_Main.cpp           # Test runner
    ├── test_Oscillator.cpp     # Oscillator unit tests
    └── test_Envelope.cpp       # Envelope unit tests
```

## Requirements

- **CMake** 3.15 or later
- **C++17** compatible compiler (MSVC, GCC, Clang)
- **JUCE Framework** 7.0+ (fetched automatically or via submodule)
- **Git** (for JUCE submodule)

## Building

### 1. Clone JUCE Framework

```bash
cd SimpleSynth
git submodule add https://github.com/juce-framework/JUCE.git libs/JUCE
git submodule update --init --recursive
```

**Alternative**: Use CMake FetchContent (add to root CMakeLists.txt):
```cmake
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
)
FetchContent_MakeAvailable(JUCE)
```

### 2. Configure and Build

#### Windows (Visual Studio)
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

#### macOS/Linux
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 3. Run Tests

```bash
cd build
ctest --verbose
# Or run directly:
./Tests/SimpleSynth_Tests  # macOS/Linux
.\Tests\Release\SimpleSynth_Tests.exe  # Windows
```

### 4. Install Plugin

After building, the VST3 plugin will be in:
- **Windows**: `build/SimpleSynth_artefacts/Release/VST3/SimpleSynth.vst3`
- **macOS**: `build/SimpleSynth_artefacts/Release/VST3/SimpleSynth.vst3`
- **Linux**: `build/SimpleSynth_artefacts/Release/VST3/SimpleSynth.vst3`

Copy to your DAW's VST3 folder:
- **Windows**: `C:\Program Files\Common Files\VST3\`
- **macOS**: `~/Library/Audio/Plug-Ins/VST3/`
- **Linux**: `~/.vst3/`

## Usage

1. Load SimpleSynth in your DAW as a VST3 instrument
2. Connect a MIDI keyboard or controller
3. Play notes (currently uses sawtooth waveform with fixed ADSR settings)
4. Enjoy the clean, band-limited sound!

## Testing

Run all unit tests:
```bash
cd build
ctest --output-on-failure
```

Individual test categories:
- **Oscillator Tests**: Waveform generation, frequency accuracy, anti-aliasing
- **Envelope Tests**: ADSR stages, gate behavior, denormal prevention

## Code Style

- **C++17** standard with modern idioms
- **Explicit types**: No auto-abuse, clear intent
- **Comments explain why**, not what
- **No mystical DSP**: Every algorithm is understandable
- **RAII and smart pointers** where appropriate

## Architecture Notes

### DSP Core Design

Inspired by Mutable Instruments:
- **State separation**: DSP objects contain no JUCE dependencies
- **Sample-rate agnostic**: All components accept sample rate as parameter
- **Block processing**: Efficient buffer-based rendering
- **Explicit state management**: Clear Init(), Reset(), Process() methods

### Oscillator

- **PolyBLEP anti-aliasing** for sawtooth and square waves
- **Pure sine wave** (no aliasing, no correction needed)
- **Normalized phase** [0, 1) for clarity
- **Frequency clamping** to valid audio range

### Envelope

- **Linear segments** (exponential curves in future phase)
- **State machine**: Idle → Attack → Decay → Sustain → Release
- **Sample-accurate** gate timing
- **Denormal prevention** to avoid CPU spikes

### Voice

- Combines oscillator + envelope
- MIDI note → frequency conversion
- Velocity scaling
- Ready for filter/modulation additions

## Roadmap

### Phase 2 (Near Future)
- [ ] Add GUI controls (waveform selector, ADSR sliders)
- [ ] Parameter automation via AudioProcessorValueTreeState
- [ ] Preset save/load system
- [ ] Oscilloscope visualizer

### Phase 3 (Advanced)
- [ ] Polyphonic voice management (4-8 voices)
- [ ] Low-pass/high-pass filters
- [ ] LFO modulation sources
- [ ] Modulation matrix routing

### Phase 4 (Experimental)
- [ ] Wavetable synthesis
- [ ] Modal synthesis (resonator bank)
- [ ] Granular synthesis
- [ ] CV-style modulation (Mutable-inspired)

## License

This project is provided as educational code. Use freely for learning and experimentation.

JUCE is licensed under GPL v3 or commercial license (see JUCE documentation).

## Contributing

This is a learning project! Feel free to:
- Report bugs or DSP issues
- Suggest architecture improvements
- Add test cases
- Propose new features

## References

- **JUCE Framework**: https://juce.com/
- **PolyBLEP**: Valimaki & Huovilainen papers on alias suppression
- **Mutable Instruments**: Open-source Eurorack firmware (design inspiration)
- **Synthesizer Architecture**: Will Pirkle - Designing Software Synthesizer Plug-Ins in C++
