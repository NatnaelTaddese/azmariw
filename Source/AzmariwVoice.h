#pragma once

#include <JuceHeader.h>
#include "AzmariwSound.h"
#include "AdsrEnvelope.h"
#include "GlideProcessor.h"

class AzmariwVoice : public juce::SynthesiserVoice
{
public:
    AzmariwVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;

    void setAdsrParameters(std::atomic<float>* attack,
                           std::atomic<float>* decay,
                           std::atomic<float>* sustain,
                           std::atomic<float>* release);

    void setGlideParameters(std::atomic<float>* enabled,
                            std::atomic<float>* time);

    void prepareToPlay(double sampleRate);

private:
    double playbackPosition = 0.0;
    float velocity = 0.0f;
    int lastPlayedNote = -1;
    bool noteWasPlaying = false;

    AdsrEnvelope adsrEnvelope;
    GlideProcessor glideProcessor;

    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* glideEnabledParam = nullptr;
    std::atomic<float>* glideTimeParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AzmariwVoice)
};
