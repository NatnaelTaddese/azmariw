#include "ParametricEq.h"

void ParametricEq::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;
    filter.prepare(spec);
    filter.reset();
}

void ParametricEq::updateParameters(float frequency, float gainDb, float q)
{
    auto gainLinear = juce::Decibels::decibelsToGain(gainDb);

    *filter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, frequency, q, gainLinear);
}

void ParametricEq::process(juce::dsp::AudioBlock<float>& block)
{
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}

void ParametricEq::reset()
{
    filter.reset();
}
