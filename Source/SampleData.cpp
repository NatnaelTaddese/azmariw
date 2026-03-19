#include "SampleData.h"

bool SampleData::loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
        return false;

    auto numSamples = static_cast<int>(reader->lengthInSamples);
    auto numChannels = static_cast<int>(reader->numChannels);

    buffer.setSize(numChannels, numSamples);
    reader->read(&buffer, 0, numSamples, 0, true, true);

    sourceSampleRate = reader->sampleRate;
    name = file.getFileNameWithoutExtension();
    filePath = file.getFullPathName();
    loopStart = 0;
    loopEnd = numSamples;

    return true;
}
