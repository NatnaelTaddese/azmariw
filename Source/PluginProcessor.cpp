#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AzmariwAudioProcessor::AzmariwAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor(BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers
    preEqFreqParam = apvts.getRawParameterValue(ParamIDs::preEqFrequency);
    preEqGainParam = apvts.getRawParameterValue(ParamIDs::preEqGain);
    preEqQParam = apvts.getRawParameterValue(ParamIDs::preEqQ);
    crossoverLowMidParam = apvts.getRawParameterValue(ParamIDs::crossoverLowMid);
    crossoverMidHighParam = apvts.getRawParameterValue(ParamIDs::crossoverMidHigh);
    driveLowParam = apvts.getRawParameterValue(ParamIDs::driveLow);
    driveMidParam = apvts.getRawParameterValue(ParamIDs::driveMid);
    driveHighParam = apvts.getRawParameterValue(ParamIDs::driveHigh);
    distortionMixParam = apvts.getRawParameterValue(ParamIDs::distortionMix);
    postEqFreqParam = apvts.getRawParameterValue(ParamIDs::postEqFrequency);
    postEqGainParam = apvts.getRawParameterValue(ParamIDs::postEqGain);
    postEqQParam = apvts.getRawParameterValue(ParamIDs::postEqQ);
    masterGainParam = apvts.getRawParameterValue(ParamIDs::masterGain);
    sampleSlotParam = apvts.getRawParameterValue(ParamIDs::sampleSlot);

    // ADSR + Glide parameter pointers
    auto* attackParam = apvts.getRawParameterValue(ParamIDs::attack);
    auto* decayParam = apvts.getRawParameterValue(ParamIDs::decay);
    auto* sustainParam = apvts.getRawParameterValue(ParamIDs::sustain);
    auto* releaseParam = apvts.getRawParameterValue(ParamIDs::release);
    auto* glideEnabledParam = apvts.getRawParameterValue(ParamIDs::glideEnabled);
    auto* glideTimeParam = apvts.getRawParameterValue(ParamIDs::glideTime);

    // Loop parameter pointers
    auto* playbackModeParam = apvts.getRawParameterValue(ParamIDs::playbackMode);
    auto* loopStartParam = apvts.getRawParameterValue(ParamIDs::loopStart);
    auto* loopEndParam = apvts.getRawParameterValue(ParamIDs::loopEnd);
    auto* loopCrossfadeParam = apvts.getRawParameterValue(ParamIDs::loopCrossfade);
    auto* loopSnapToZeroParam = apvts.getRawParameterValue(ParamIDs::loopSnapToZero);

    // Add voices
    for (int i = 0; i < 8; ++i)
    {
        auto* voice = new AzmariwVoice();
        voice->setAdsrParameters(attackParam, decayParam, sustainParam, releaseParam);
        voice->setGlideParameters(glideEnabledParam, glideTimeParam);
        voice->setLoopParameters(playbackModeParam, loopStartParam, loopEndParam, loopCrossfadeParam, loopSnapToZeroParam);
        synthesiser.addVoice(voice);
    }

    // Add initial sound (will be updated when samples are loaded)
    auto* defaultSample = sampleManager.getSampleData(0);
    synthesiser.addSound(new AzmariwSound(defaultSample));

    // Try to load preset samples from Samples directory next to plugin
    auto pluginFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    auto samplesDir = pluginFile.getParentDirectory().getChildFile("Samples");
    sampleManager.loadPresetSamples(samplesDir);
}

AzmariwAudioProcessor::~AzmariwAudioProcessor()
{
}

//==============================================================================
const juce::String AzmariwAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AzmariwAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AzmariwAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AzmariwAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AzmariwAudioProcessor::getTailLengthSeconds() const
{
    // Return max release time for proper tail handling
    auto* releaseParam = apvts.getRawParameterValue(ParamIDs::release);
    if (releaseParam != nullptr)
        return static_cast<double>(releaseParam->load());

    return 10.0;
}

int AzmariwAudioProcessor::getNumPrograms()
{
    return 1;
}

int AzmariwAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AzmariwAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AzmariwAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AzmariwAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AzmariwAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    synthesiser.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synthesiser.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<AzmariwVoice*>(synthesiser.getVoice(i)))
            voice->prepareToPlay(sampleRate);
    }

    preEq.prepare(spec);
    multibandDistortion.prepare(spec);
    postEq.prepare(spec);
    masterGain.prepare(spec);
    masterGain.setRampDurationSeconds(0.02);
}

void AzmariwAudioProcessor::releaseResources()
{
    preEq.reset();
    multibandDistortion.reset();
    postEq.reset();
    masterGain.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AzmariwAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AzmariwAudioProcessor::updateFxParameters()
{
    // Update the active sound's sample data based on selected slot
    int slotIndex = static_cast<int>(sampleSlotParam->load());
    auto* sampleData = sampleManager.getSampleData(slotIndex);

    if (synthesiser.getNumSounds() > 0)
    {
        if (auto* sound = dynamic_cast<AzmariwSound*>(synthesiser.getSound(0).get()))
            sound->setSampleData(sampleData);
    }

    // Pre-EQ
    preEq.updateParameters(preEqFreqParam->load(),
                           preEqGainParam->load(),
                           preEqQParam->load());

    // Distortion
    multibandDistortion.updateParameters(crossoverLowMidParam->load(),
                                          crossoverMidHighParam->load(),
                                          driveLowParam->load(),
                                          driveMidParam->load(),
                                          driveHighParam->load(),
                                          distortionMixParam->load());

    // Post-EQ
    postEq.updateParameters(postEqFreqParam->load(),
                            postEqGainParam->load(),
                            postEqQParam->load());

    // Master gain
    masterGain.setGainDecibels(masterGainParam->load());
}

void AzmariwAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update all FX parameters from APVTS
    updateFxParameters();

    // Clear and render synth voices
    buffer.clear();
    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Global FX chain
    juce::dsp::AudioBlock<float> block(buffer);

    // Pre-EQ
    preEq.process(block);

    // Multiband distortion
    multibandDistortion.process(buffer);

    // Post-EQ
    postEq.process(block);

    // Master gain
    juce::dsp::ProcessContextReplacing<float> gainContext(block);
    masterGain.process(gainContext);
}

//==============================================================================
bool AzmariwAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AzmariwAudioProcessor::createEditor()
{
    return new AzmariwAudioProcessorEditor(*this);
}

//==============================================================================
void AzmariwAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    sampleManager.saveState(state);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AzmariwAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        auto state = juce::ValueTree::fromXml(*xml);
        apvts.replaceState(state);
        sampleManager.loadState(state);
    }
}

//==============================================================================
bool AzmariwAudioProcessor::loadSampleFromFile(int slot, const juce::File& file)
{
    return sampleManager.loadUserSample(slot, file);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AzmariwAudioProcessor();
}
