#pragma once

#include <JuceHeader.h>

class GlideProcessor
{
public:
    GlideProcessor() = default;

    void prepare(double sampleRate);
    void setGlideTime(float timeSeconds);
    void setTargetFrequency(float freq, bool shouldGlide);
    float getNextFrequency();
    float getCurrentFrequency() const { return currentFrequency; }
    void reset();

private:
    float currentFrequency = 440.0f;
    float targetFrequency = 440.0f;
    float glideRate = 1.0f;
    double currentSampleRate = 44100.0;
    bool active = false;
};
