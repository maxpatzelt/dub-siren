# SimpleSynth - Build & Test Guide

## Quick Start (After Build Tools Install)

### Option 1: Using Visual Studio Developer Command Prompt

1. **Open "Developer Command Prompt for VS 2022"** (search in Start menu)

2. **Navigate and build**:
   ```cmd
   cd C:\Users\maxpa\SimpleSynth
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **Run tests**:
   ```cmd
   .\Tests\Release\SimpleSynth_Tests.exe
   ```

### Option 2: Using PowerShell with vcvars

1. **Initialize Visual Studio environment**:
   ```powershell
   cd C:\Users\maxpa\SimpleSynth
   & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
   ```

2. **Build**:
   ```powershell
   Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **Run tests**:
   ```powershell
   .\Tests\Release\SimpleSynth_Tests.exe
   ```

---

## Testing the Synthesizer

### 1. Unit Tests (Automated)

**Run all tests**:
```powershell
cd C:\Users\maxpa\SimpleSynth\build
.\Tests\Release\SimpleSynth_Tests.exe
```

**Expected output**:
```
JUCE v7.x.x
Running test: Oscillator Tests...
  Initialization                          [OK]
  Frequency Setting                       [OK]
  Waveform Output Validity                [OK]
  Phase Continuity                        [OK]
  Sample Rate Change                      [OK]
  Zero Crossings                          [OK]

Running test: Envelope Tests...
  Initialization                          [OK]
  Attack Stage                            [OK]
  Decay Stage                             [OK]
  Sustain Stage                           [OK]
  Release Stage                           [OK]
  Full ADSR Cycle                         [OK]
  Retrigger During Release                [OK]
  No Denormals                            [OK]

===========================================
Test Summary:
  Total Tests: 2
  Passes: 14
  Failures: 0
===========================================
```

### 2. Install VST3 Plugin

**Find the built plugin**:
```powershell
cd C:\Users\maxpa\SimpleSynth\build
dir SimpleSynth_artefacts\Release\VST3\SimpleSynth.vst3 -Recurse
```

**Install to system VST3 folder**:
```powershell
$vst3Source = "C:\Users\maxpa\SimpleSynth\build\SimpleSynth_artefacts\Release\VST3\SimpleSynth.vst3"
$vst3Dest = "$env:CommonProgramFiles\VST3\"

# Copy plugin
Copy-Item -Path $vst3Source -Destination $vst3Dest -Recurse -Force

Write-Host "SimpleSynth installed to: $vst3Dest"
```

### 3. Test in a DAW

#### Free DAW Options:
- **Reaper** (free trial, fully functional): https://www.reaper.fm/download.php
- **Cakewalk** (completely free): https://www.bandlab.com/products/cakewalk
- **Tracktion Waveform Free**: https://www.tracktion.com/products/waveform-free

#### Testing Steps:

1. **Open your DAW** (e.g., Reaper)

2. **Create a new project / track**

3. **Add SimpleSynth as instrument**:
   - In Reaper: Insert → Virtual Instrument → VST3 → SimpleSynth

4. **Arm track for recording** and enable input monitoring

5. **Play MIDI notes** (keyboard or virtual keyboard):
   - You should hear a **sawtooth wave** tone
   - Note on triggers the envelope **attack**
   - Note off triggers **release**

6. **Test features**:
   - Play different notes (pitch tracking works)
   - Try velocity sensitivity (louder = higher velocity)
   - Hold notes (sustain stage)
   - Quick retriggering (envelope restarts correctly)

### 4. Manual Audio Test (Without DAW)

If you want to verify audio output programmatically:

**Create test MIDI file player** (optional advanced step):
```cpp
// Add to Tests directory as test_AudioOutput.cpp
// This would require JUCE's AudioDeviceManager setup
// See README for future phase 2 expansion
```

---

## Troubleshooting

### Tests Fail

**Check error messages**:
```powershell
.\Tests\Release\SimpleSynth_Tests.exe --verbose
```

**Common issues**:
- Sample rate mismatch: Verify Init() calls in tests
- Denormal problems: Check PreventDenormal() function
- Phase errors: Verify WrapPhase() usage

### Plugin Not Found in DAW

**Verify VST3 installation**:
```powershell
ls "$env:CommonProgramFiles\VST3\SimpleSynth.vst3"
```

**Rescan plugins** in your DAW:
- Reaper: Preferences → Plug-ins → VST → Re-scan
- Cakewalk: Utilities → Plug-in Manager → Scan for Plug-ins

### No Sound in DAW

1. **Check MIDI input**: Verify track is armed and receiving MIDI
2. **Check output routing**: Ensure track outputs to master
3. **Check plugin instance**: SimpleSynth GUI should show "SimpleSynth" title
4. **Try different notes**: MIDI note 60 (C4) at full velocity

### Build Errors

**Missing JUCE**:
```powershell
cd C:\Users\maxpa\SimpleSynth
git submodule update --init --recursive
```

**CMake errors**:
```powershell
# Clean and rebuild
Remove-Item -Recurse -Force build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## Performance Validation

### CPU Usage Test

In your DAW:
1. Load SimpleSynth on a track
2. Play sustained notes
3. Monitor CPU usage (should be <1% for single voice)

### Audio Quality Test

1. **Record output** to audio file
2. **Inspect waveform**: Should see smooth sawtooth
3. **Frequency analysis**: Fundamental + harmonics, minimal aliasing
4. **Envelope shape**: Clean attack/release curves

### Stress Test

```cpp
// Future: Add polyphony and test with many simultaneous notes
// Current monophonic design should be rock-solid stable
```

---

## Next Steps

Once basic testing passes:

### Phase 2 Enhancements
- [ ] Add GUI controls for waveform selection
- [ ] Add ADSR sliders
- [ ] Implement parameter automation
- [ ] Add preset system

### Advanced Testing
- [ ] Frequency accuracy measurement (FFT)
- [ ] THD+N analysis
- [ ] Aliasing measurement (analyze harmonics above Nyquist)
- [ ] Envelope timing precision (compare to spec)

---

## Expected Test Results Summary

✅ **All unit tests pass** (14/14)
✅ **Plugin loads in DAW** without errors
✅ **MIDI notes produce sound** (sawtooth wave)
✅ **Pitch tracking accurate** across full keyboard range
✅ **Envelope responds** to note on/off
✅ **No clicks or pops** during note transitions
✅ **Stable CPU usage** (<1% single voice)

If all checks pass: **Congratulations! You have a working software synthesizer!** 🎉
