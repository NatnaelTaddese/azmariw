#include "AzmariwVoice.h"

AzmariwVoice::AzmariwVoice()
{
}

bool AzmariwVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<AzmariwSound*>(sound) != nullptr;
}

void AzmariwVoice::startNote(int midiNoteNumber, float vel,
                              juce::SynthesiserSound* sound,
                              int /*currentPitchWheelPosition*/)
{
    velocity = vel;
    playbackPosition = 0.0;

    auto* azmariwSound = dynamic_cast<AzmariwSound*>(sound);
    if (azmariwSound == nullptr)
        return;

    auto* sampleData = azmariwSound->getSampleData();
    if (sampleData == nullptr || !sampleData->isLoaded())
        return;

    float targetFreq = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));

    bool shouldGlide = noteWasPlaying
                       && glideEnabledParam != nullptr
                       && glideEnabledParam->load() >= 0.5f;

    if (glideTimeParam != nullptr)
        glideProcessor.setGlideTime(glideTimeParam->load());

    glideProcessor.setTargetFrequency(targetFreq, shouldGlide);

    if (attackParam != nullptr)
        adsrEnvelope.updateParameters(attackParam, decayParam, sustainParam, releaseParam);

    adsrEnvelope.noteOn();
    noteWasPlaying = true;
    lastPlayedNote = midiNoteNumber;
}

void AzmariwVoice::stopNote(float /*vel*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsrEnvelope.noteOff();
    }
    else
    {
        adsrEnvelope.reset();
        clearCurrentNote();
        noteWasPlaying = false;
        playbackPosition = 0.0;
    }
}

void AzmariwVoice::pitchWheelMoved(int /*newPitchWheelValue*/)
{
}

void AzmariwVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/)
{
}

void AzmariwVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                    int startSample, int numSamples)
{
    auto* sound = dynamic_cast<AzmariwSound*>(getCurrentlyPlayingSound().get());
    if (sound == nullptr)
        return;

    auto* sampleData = sound->getSampleData();
    if (sampleData == nullptr || !sampleData->isLoaded())
        return;

    const auto& sampleBuffer = sampleData->getBuffer();
    auto sampleLength = sampleBuffer.getNumSamples();
    auto numChannels = sampleBuffer.getNumChannels();
    auto sourceSampleRate = sampleData->getSourceSampleRate();
    auto rootNote = sampleData->getRootNote();
    bool loopEnabled = sampleData->isLoopEnabled();
    int loopStart = sampleData->getLoopStart();
    int loopEnd = sampleData->getLoopEnd();

    if (loopEnd <= 0)
        loopEnd = sampleLength;

    if (attackParam != nullptr)
        adsrEnvelope.updateParameters(attackParam, decayParam, sustainParam, releaseParam);

    float rootFreq = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(rootNote));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (!adsrEnvelope.isActive())
        {
            clearCurrentNote();
            noteWasPlaying = false;
            break;
        }

        float currentFreq = glideProcessor.getNextFrequency();
        double pitchRatio = (currentFreq / rootFreq) * (sourceSampleRate / getSampleRate());

        auto pos = static_cast<int>(playbackPosition);
        auto frac = static_cast<float>(playbackPosition - pos);

        float envelopeValue = adsrEnvelope.getNextSample();

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            int srcChannel = juce::jmin(channel, numChannels - 1);

            float sample0 = 0.0f;
            float sample1 = 0.0f;

            if (pos >= 0 && pos < sampleLength)
                sample0 = sampleBuffer.getSample(srcChannel, pos);
            if (pos + 1 >= 0 && pos + 1 < sampleLength)
                sample1 = sampleBuffer.getSample(srcChannel, pos + 1);

            float interpolated = sample0 + frac * (sample1 - sample0);
            outputBuffer.addSample(channel, startSample + sample,
                                   interpolated * velocity * envelopeValue);
        }

        playbackPosition += pitchRatio;

        if (loopEnabled)
        {
            if (playbackPosition >= loopEnd)
                playbackPosition = loopStart + std::fmod(playbackPosition - loopStart,
                                                          loopEnd - loopStart);
        }
        else
        {
            if (playbackPosition >= sampleLength)
            {
                adsrEnvelope.noteOff();
            }
        }
    }
}

void AzmariwVoice::setAdsrParameters(std::atomic<float>* attack,
                                      std::atomic<float>* decay,
                                      std::atomic<float>* sustain,
                                      std::atomic<float>* release)
{
    attackParam = attack;
    decayParam = decay;
    sustainParam = sustain;
    releaseParam = release;
}

void AzmariwVoice::setGlideParameters(std::atomic<float>* enabled,
                                       std::atomic<float>* time)
{
    glideEnabledParam = enabled;
    glideTimeParam = time;
}

void AzmariwVoice::prepareToPlay(double sampleRate)
{
    adsrEnvelope.setSampleRate(sampleRate);
    glideProcessor.prepare(sampleRate);
}
