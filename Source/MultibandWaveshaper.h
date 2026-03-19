#pragma once

#include <JuceHeader.h>

class MultibandWaveshaper
{
public:
    MultibandWaveshaper() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void updateParameters(float crossoverLowMid, float crossoverMidHigh,
                          float driveLow, float driveMid, float driveHigh,
                          float mix, int distortionType);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

private:
    static float waveshape(float input, float drive, int distortionType);

    juce::dsp::LinkwitzRileyFilter<float> lowMidLP, lowMidHP;
    juce::dsp::LinkwitzRileyFilter<float> midHighLP, midHighHP;

    juce::AudioBuffer<float> lowBand, midBand, highBand, dryBuffer;

    float currentDriveLow = 0.0f;
    float currentDriveMid = 0.0f;
    float currentDriveHigh = 0.0f;
    float currentMix = 0.5f;
    int currentDistortionType = 0;

    int numChannels = 2;
};
