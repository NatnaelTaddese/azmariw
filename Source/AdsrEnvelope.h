#pragma once

#include <JuceHeader.h>

class AdsrEnvelope
{
public:
    AdsrEnvelope() = default;

    void setSampleRate(double sampleRate) { adsr.setSampleRate(sampleRate); }

    void updateParameters(std::atomic<float>* attack,
                          std::atomic<float>* decay,
                          std::atomic<float>* sustain,
                          std::atomic<float>* release)
    {
        juce::ADSR::Parameters params;
        params.attack = attack->load();
        params.decay = decay->load();
        params.sustain = sustain->load();
        params.release = release->load();
        adsr.setParameters(params);
    }

    void noteOn() { adsr.noteOn(); }
    void noteOff() { adsr.noteOff(); }
    float getNextSample() { return adsr.getNextSample(); }
    bool isActive() const { return adsr.isActive(); }
    void reset() { adsr.reset(); }

private:
    juce::ADSR adsr;
};
