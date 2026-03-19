#pragma once

#include <JuceHeader.h>
#include "SampleData.h"

class AzmariwSound : public juce::SynthesiserSound
{
public:
    AzmariwSound(SampleData* sample) : sampleData(sample) {}

    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }

    SampleData* getSampleData() const { return sampleData; }
    void setSampleData(SampleData* sample) { sampleData = sample; }

private:
    SampleData* sampleData = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AzmariwSound)
};
