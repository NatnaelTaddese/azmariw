#pragma once

#include <JuceHeader.h>

class ParametricEq
{
public:
    ParametricEq() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void updateParameters(float frequency, float gainDb, float q);
    void process(juce::dsp::AudioBlock<float>& block);
    void reset();

private:
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                    juce::dsp::IIR::Coefficients<float>> filter;
    double currentSampleRate = 44100.0;
};
