# Agent Guidelines for Azmariw

## Project Overview
Azmariw is a JUCE-based VST3 audio plugin synthesizer. Built with C++17/20 and JUCE framework.
Project configuration managed via `Azmariw.jucer` (Projucer).

---

## Build Commands

### Generate Visual Studio Project
```bash
# Windows: Open Azmariw.jucer in Projucer app
# or use JUCE's command-line tools:
juce::Projucer --resave Azmariw.jucer
```

### Build with Visual Studio 2022
```bash
# Open in VS2022
Azmariw/Builds/VisualStudio2022/Azmariw.sln

# Or build from command line
msbuild Azmariw/Builds/VisualStudio2022/Azmariw.vcxproj /p:Configuration=Release
```

### Build Configurations
- **Debug**: Debug builds with symbols (`isDebug="1"`)
- **Release**: Optimized release builds (`isDebug="0"`)

---

## Testing
**No automated test framework is currently set up.**
If adding tests:
```bash
# Consider using Catch2 or doctest for unit tests
# Build tests as separate target from Projucer
```

---

## Code Style Guidelines

### Files and Organization
- Header files: `.h` with `#pragma once`
- Implementation: `.cpp` files
- Each class in its own header/implementation pair
- Use `//==============` separators for major sections

### Naming Conventions
| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase + Audio/AudioProcessor suffix | `AzmariwAudioProcessor` |
| Methods | camelCase | `processBlock`, `prepareToPlay` |
| Variables | camelCase | `totalNumInputChannels` |
| Constants | camelCase | `sampleRate`, `samplesPerBlock` |
| JUCE Classes | `juce::` prefix | `juce::AudioBuffer`, `juce::MidiBuffer` |
| Member variables | No underscore suffix (JUCE style) | `audioProcessor` |
| Private sections | `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` | - |

### Formatting
- 4-space indentation (tabs)
- Braces on same line (K&R style):
  ```cpp
  if (condition) {
      // code
  } else {
      // code
  }
  ```
- Spaces around operators: `channel + 1`, not `channel+1`
- No spaces in function calls: `getNumSamples()`, not `getNumSamples ()`

### Includes
```cpp
#include <JuceHeader.h>  // JUCE framework
#include "PluginProcessor.h"  // Local headers (quoted)
```
- Local includes use `" "` quotes
- System/JUCE includes use `< >` angle brackets
- Order: JuceHeader first, then local includes

### JUCE-Specific Patterns
```cpp
// Use juce:: namespace prefix
juce::AudioBuffer<float> buffer;
juce::MidiBuffer midiMessages;

// Scoped resource management
juce::ScopedNoDenormals noDenormals;

// Editor factory
juce::AudioProcessorEditor* createEditor() override;

// Plugin entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
```

### Error Handling
- No exceptions (C++ exception handling in JUCE plugins is discouraged)
- Use `ignoreUnused()` for intentionally unused parameters:
  ```cpp
  juce::ignoreUnused(layouts);
  ```

### Preprocessor
- Use JUCE plugin defines for conditional compilation:
  ```cpp
  #if JucePlugin_WantsMidiInput
  #endif
  ```

---

## Architecture

### Audio Processor Chain
1. `AudioProcessor` (PluginProcessor) - Core DSP, `processBlock()`
2. `AudioProcessorEditor` (PluginEditor) - GUI visualization

### Class Hierarchy
```
juce::AudioProcessor
└── AzmariwAudioProcessor

juce::AudioProcessorEditor
└── AzmariwAudioProcessorEditor
```

### Key Override Methods
- `prepareToPlay()` - Initialize DSP
- `releaseResources()` - Cleanup
- `processBlock()` - Audio/MIDI processing
- `createEditor()` - Return GUI instance

---

## Development Workflow

1. **Edit source files** in `Azmariw/Source/`
2. **Regenerate VS project** if adding/removing files:
   - Edit `Azmariw.jucer`
   - Resave project via Projucer
3. **Build** in Visual Studio 2022
4. **Test** in DAW (VST3 host)

---

## Important Notes
- Always include `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` in JUCE classes
- Use `juce::` namespace for all JUCE types
- Call `juce::ScopedNoDenormals` at start of `processBlock()`
- Clear unused output channels to avoid feedback
- Use `auto` for JUCE types when type is obvious
