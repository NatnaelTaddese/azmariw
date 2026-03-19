#pragma once

#include <JuceHeader.h>
#include "SampleData.h"

class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay();

    void setSampleData(const SampleData* sample);
    void setLoopParameters(float loopStartNorm, float loopEndNorm, float crossfadeMs);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void rebuildWaveformPath();

    const SampleData* currentSample = nullptr;
    juce::Path waveformPath;
    bool pathDirty = true;

    float loopStart = 0.0f;
    float loopEnd = 1.0f;
    float crossfadeMs = 20.0f;

    // Cached thumbnail data for efficient drawing
    std::vector<float> waveformPeaks;
    int cachedWidth = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
