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

int SampleData::findNearestZeroCrossing(int sampleIndex, int searchRadius) const
{
    int numSamples = buffer.getNumSamples();
    if (numSamples < 2 || sampleIndex < 0 || sampleIndex >= numSamples)
        return sampleIndex;

    const float* data = buffer.getReadPointer(0); // use first channel

    int bestIndex = sampleIndex;
    int bestDistance = searchRadius + 1;

    for (int offset = 0; offset <= searchRadius; ++offset)
    {
        // Search forward
        int fwd = sampleIndex + offset;
        if (fwd < numSamples - 1)
        {
            if ((data[fwd] >= 0.0f && data[fwd + 1] < 0.0f) ||
                (data[fwd] < 0.0f && data[fwd + 1] >= 0.0f) ||
                data[fwd] == 0.0f)
            {
                if (offset < bestDistance)
                {
                    bestDistance = offset;
                    // Pick the sample closer to zero
                    bestIndex = (std::abs(data[fwd]) <= std::abs(data[fwd + 1])) ? fwd : fwd + 1;
                }
                break; // forward search found one, no need to continue forward
            }
        }

        // Search backward
        if (offset > 0)
        {
            int bwd = sampleIndex - offset;
            if (bwd >= 0 && bwd < numSamples - 1)
            {
                if ((data[bwd] >= 0.0f && data[bwd + 1] < 0.0f) ||
                    (data[bwd] < 0.0f && data[bwd + 1] >= 0.0f) ||
                    data[bwd] == 0.0f)
                {
                    if (offset < bestDistance)
                    {
                        bestDistance = offset;
                        bestIndex = (std::abs(data[bwd]) <= std::abs(data[bwd + 1])) ? bwd : bwd + 1;
                    }
                    break; // backward search found one
                }
            }
        }
    }

    return juce::jlimit(0, numSamples - 1, bestIndex);
}
