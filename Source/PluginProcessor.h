#pragma once

#include <JuceHeader.h>
#include "SampleManager.h"
#include "AzmariwSound.h"
#include "AzmariwVoice.h"
#include "ParametricEq.h"
#include "MultibandWaveshaper.h"
#include "PluginParameters.h"

class AzmariwAudioProcessor : public juce::AudioProcessor
{
public:
    AzmariwAudioProcessor();
    ~AzmariwAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool loadSampleFromFile(int slot, const juce::File& file);
    SampleManager& getSampleManager() { return sampleManager; }

    juce::AudioProcessorValueTreeState apvts;

private:
    void updateFxParameters();

    SampleManager sampleManager;
    juce::Synthesiser synthesiser;

    ParametricEq preEq;
    MultibandWaveshaper multibandDistortion;
    ParametricEq postEq;
    juce::dsp::Gain<float> masterGain;

    // Cached parameter pointers
    std::atomic<float>* preEqFreqParam = nullptr;
    std::atomic<float>* preEqGainParam = nullptr;
    std::atomic<float>* preEqQParam = nullptr;
    std::atomic<float>* crossoverLowMidParam = nullptr;
    std::atomic<float>* crossoverMidHighParam = nullptr;
    std::atomic<float>* driveLowParam = nullptr;
    std::atomic<float>* driveMidParam = nullptr;
    std::atomic<float>* driveHighParam = nullptr;
    std::atomic<float>* distortionMixParam = nullptr;
    std::atomic<float>* postEqFreqParam = nullptr;
    std::atomic<float>* postEqGainParam = nullptr;
    std::atomic<float>* postEqQParam = nullptr;
    std::atomic<float>* masterGainParam = nullptr;
    std::atomic<float>* sampleSlotParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AzmariwAudioProcessor)
};
