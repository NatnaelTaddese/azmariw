# Azmariw

Azmariw is a JUCE-based VST3 audio plugin synthesizer focused on creating authentic Ethiopian instrument sounds. The project aims to bring the character, tone, and musical feel of traditional Ethiopian instruments into modern digital audio workstations.

## Prerequisites

- **Visual Studio 2022** (or compatible C++20 compiler)
- **JUCE Framework** (v7.0+)
- **Projucer** (included with JUCE)

## Setup

### 1. Clone or Download JUCE

Download JUCE from [https://github.com/juce-framework/JUCE](https://github.com/juce-framework/JUCE) and place it so that the folder structure matches:

```
C:\Dev\vst\
├── src\
│   └── JUCE\
│       └── modules\    <-- JUCE modules here
└── azmariw\
    └── Azmariw\
```

The module path in `Azmariw.jucer` is configured as `../../../../src/JUCE/modules` relative to `Azmariw/Builds/VisualStudio2022/`.

### 2. Generate Visual Studio Project

#### Option A: Using Projucer GUI
1. Open `Azmariw.jucer` in the Projucer application
2. Go to **File > Export > Visual Studio 2022**
3. Click "Export"

#### Option B: Command Line
```bash
juce::Projucer --resave Azmariw.jucer
```

This generates `Azmariw/Builds/VisualStudio2022/Azmariw.sln`.

### 3. Setup Test Host (Optional)

To test the plugin during development:

1. Download **AudioPluginHost** from [JUCE GitHub](https://github.com/juce-framework/JUCE/tree/master/extras/AudioPluginHost)
2. Build AudioPluginHost as a standalone executable
3. Place `AudioPluginHost.exe` in:

```
Azmariw/
└── Deps/
    └── Debug/
        └── AudioHost/
            └── AudioPluginHost.exe
```

The `Filtergraph/` folder in `Deps/Debug/` can hold saved filter graph presets (`.filtergraph` files).

### 4. Build the Plugin

Open the solution in Visual Studio:
```
Azmariw/Builds/VisualStudio2022/Azmariw.sln
```

**Build Configurations:**
- **Debug** - Development build with debug symbols
- **Release** - Optimized production build

**Build Output:**
- VST3 plugin: `Azmariw/Builds/VisualStudio2022/x64/Release (or Debug)/Azmariw.vst3`

### 5. Install the Plugin

Copy the VST3 bundle to your DAW's plugin directory:

| Location | Path |
|----------|------|
| System | `C:\Program Files\Common Files\VST3\` |
| Per-user | `%LOCALAPPDATA%\Programs\VST3\` |

Or create a symbolic link for development:
```cmd
mklink /D "%LOCALAPPDATA%\Programs\VST3\Azmariw.vst3" "C:\Dev\vst\azmariw\Azmariw\Builds\VisualStudio2022\x64\Release\Azmariw.vst3"
```

## Project Structure

```
Azmariw/
├── Azmariw.jucer       # Projucer project file
├── JuceLibraryCode/    # Auto-generated JUCE headers
├── Source/             # Plugin source code
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
├── Builds/             # Generated build files
│   └── VisualStudio2022/
└── Deps/               # Dependencies
    └── Debug/
        └── AudioHost/  # AudioPluginHost.exe for testing
```

## Development

After modifying source files, regenerate the VS project via Projucer if you add/remove files.

## Resources

- [JUCE Documentation](https://docs.juce.com/)
- [JUCE Forum](https://forum.juce.com/)
