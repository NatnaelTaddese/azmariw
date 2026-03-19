#pragma once

#include <JuceHeader.h>

class SampleData
{
public:
    SampleData() = default;

    bool loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager);

    const juce::AudioBuffer<float>& getBuffer() const { return buffer; }
    double getSourceSampleRate() const { return sourceSampleRate; }
    int getRootNote() const { return rootNote; }
    void setRootNote(int note) { rootNote = note; }
    int getLoopStart() const { return loopStart; }
    int getLoopEnd() const { return loopEnd; }
    void setLoopPoints(int start, int end) { loopStart = start; loopEnd = end; }
    bool isLoopEnabled() const { return loopEnabled; }
    void setLoopEnabled(bool enabled) { loopEnabled = enabled; }
    const juce::String& getName() const { return name; }
    const juce::String& getFilePath() const { return filePath; }
    bool isLoaded() const { return buffer.getNumSamples() > 0; }

private:
    juce::AudioBuffer<float> buffer;
    double sourceSampleRate = 44100.0;
    int rootNote = 60;
    int loopStart = 0;
    int loopEnd = 0;
    bool loopEnabled = false;
    juce::String name;
    juce::String filePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleData)
};
