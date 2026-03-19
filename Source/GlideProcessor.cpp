#include "GlideProcessor.h"

void GlideProcessor::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
}

void GlideProcessor::setGlideTime(float timeSeconds)
{
    if (timeSeconds <= 0.001f)
        glideRate = 1.0f;
    else
        glideRate = 1.0f - std::exp(-1.0f / (timeSeconds * static_cast<float>(currentSampleRate)));
}

void GlideProcessor::setTargetFrequency(float freq, bool shouldGlide)
{
    targetFrequency = freq;

    if (!shouldGlide)
        currentFrequency = freq;

    active = shouldGlide;
}

float GlideProcessor::getNextFrequency()
{
    if (active && currentFrequency != targetFrequency)
    {
        currentFrequency += (targetFrequency - currentFrequency) * glideRate;

        if (std::abs(targetFrequency - currentFrequency) < 0.01f)
        {
            currentFrequency = targetFrequency;
            active = false;
        }
    }

    return currentFrequency;
}

void GlideProcessor::reset()
{
    currentFrequency = 440.0f;
    targetFrequency = 440.0f;
    active = false;
}
