#pragma once

#include <JuceHeader.h>
#include "SampleData.h"

class SampleManager
{
public:
    static constexpr int maxSlots = 8;

    SampleManager();

    void loadPresetSamples(const juce::File& sampleDirectory);
    bool loadUserSample(int slot, const juce::File& file);

    SampleData* getSampleData(int slot);
    const SampleData* getSampleData(int slot) const;

    void saveState(juce::ValueTree& state) const;
    void loadState(const juce::ValueTree& state);

    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    juce::AudioFormatManager formatManager;
    std::array<std::unique_ptr<SampleData>, maxSlots> slots;
    juce::SpinLock sampleLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleManager)
};
