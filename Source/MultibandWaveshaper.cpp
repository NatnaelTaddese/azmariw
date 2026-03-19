#include "MultibandWaveshaper.h"

void MultibandWaveshaper::prepare(const juce::dsp::ProcessSpec& spec)
{
    numChannels = static_cast<int>(spec.numChannels);
    auto maxBlockSize = static_cast<int>(spec.maximumBlockSize);

    lowMidLP.prepare(spec);
    lowMidHP.prepare(spec);
    midHighLP.prepare(spec);
    midHighHP.prepare(spec);

    lowMidLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowMidHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    midHighLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    midHighHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    lowBand.setSize(numChannels, maxBlockSize);
    midBand.setSize(numChannels, maxBlockSize);
    highBand.setSize(numChannels, maxBlockSize);
    dryBuffer.setSize(numChannels, maxBlockSize);

    reset();
}

void MultibandWaveshaper::updateParameters(float crossoverLowMid, float crossoverMidHigh,
                                            float driveLow, float driveMid, float driveHigh,
                                            float mix)
{
    lowMidLP.setCutoffFrequency(crossoverLowMid);
    lowMidHP.setCutoffFrequency(crossoverLowMid);
    midHighLP.setCutoffFrequency(crossoverMidHigh);
    midHighHP.setCutoffFrequency(crossoverMidHigh);

    currentDriveLow = driveLow;
    currentDriveMid = driveMid;
    currentDriveHigh = driveHigh;
    currentMix = mix;
}

float MultibandWaveshaper::waveshape(float input, float drive)
{
    if (drive <= 0.001f)
        return input;

    // Map 0-1 drive to 1-50 multiplier (exponential for musical feel)
    float driveAmount = 1.0f + drive * drive * 49.0f;
    return std::tanh(driveAmount * input);
}

void MultibandWaveshaper::process(juce::AudioBuffer<float>& buffer)
{
    auto numSamples = buffer.getNumSamples();
    auto channels = buffer.getNumChannels();

    // Save dry signal
    for (int ch = 0; ch < channels; ++ch)
        dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

    // Copy input to band buffers for splitting
    for (int ch = 0; ch < channels; ++ch)
    {
        lowBand.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        highBand.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    // Split into low and mid+high
    auto lowBlock = juce::dsp::AudioBlock<float>(lowBand).getSubBlock(0, static_cast<size_t>(numSamples));
    auto highBlock = juce::dsp::AudioBlock<float>(highBand).getSubBlock(0, static_cast<size_t>(numSamples));

    lowMidLP.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
    lowMidHP.process(juce::dsp::ProcessContextReplacing<float>(highBlock));

    // Copy mid+high to mid buffer, then split
    for (int ch = 0; ch < channels; ++ch)
        midBand.copyFrom(ch, 0, highBand, ch, 0, numSamples);

    auto midBlock = juce::dsp::AudioBlock<float>(midBand).getSubBlock(0, static_cast<size_t>(numSamples));
    highBlock = juce::dsp::AudioBlock<float>(highBand).getSubBlock(0, static_cast<size_t>(numSamples));

    midHighLP.process(juce::dsp::ProcessContextReplacing<float>(midBlock));
    midHighHP.process(juce::dsp::ProcessContextReplacing<float>(highBlock));

    // Apply waveshaping to each band and sum into output
    buffer.clear();

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* outData = buffer.getWritePointer(ch);
        auto* lowData = lowBand.getReadPointer(ch);
        auto* midData = midBand.getReadPointer(ch);
        auto* highData = highBand.getReadPointer(ch);
        auto* dryData = dryBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float wet = waveshape(lowData[i], currentDriveLow)
                      + waveshape(midData[i], currentDriveMid)
                      + waveshape(highData[i], currentDriveHigh);

            outData[i] = dryData[i] * (1.0f - currentMix) + wet * currentMix;
        }
    }
}

void MultibandWaveshaper::reset()
{
    lowMidLP.reset();
    lowMidHP.reset();
    midHighLP.reset();
    midHighHP.reset();

    lowBand.clear();
    midBand.clear();
    highBand.clear();
    dryBuffer.clear();
}
