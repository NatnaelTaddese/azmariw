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
    hasCompletedFirstPass = false;

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
        hasCompletedFirstPass = false;
        playbackPosition = 0.0;
    }
}

void AzmariwVoice::pitchWheelMoved(int /*newPitchWheelValue*/)
{
}

void AzmariwVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/)
{
}

float AzmariwVoice::getInterpolatedSample(const juce::AudioBuffer<float>& buffer,
                                           int channel, double position, int numSamples) const
{
    auto pos = static_cast<int>(position);
    auto frac = static_cast<float>(position - pos);

    float s0 = 0.0f;
    float s1 = 0.0f;

    if (pos >= 0 && pos < numSamples)
        s0 = buffer.getSample(channel, pos);
    if (pos + 1 >= 0 && pos + 1 < numSamples)
        s1 = buffer.getSample(channel, pos + 1);

    return s0 + frac * (s1 - s0);
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

    // Read loop parameters from APVTS
    bool loopEnabled = (playbackModeParam != nullptr && playbackModeParam->load() >= 0.5f);
    float loopStartNorm = (loopStartParam != nullptr) ? loopStartParam->load() : 0.0f;
    float loopEndNorm = (loopEndParam != nullptr) ? loopEndParam->load() : 1.0f;
    float crossfadeMs = (loopCrossfadeParam != nullptr) ? loopCrossfadeParam->load() : 20.0f;
    bool snapEnabled = (loopSnapToZeroParam != nullptr && loopSnapToZeroParam->load() >= 0.5f);

    // Convert normalized positions to absolute sample indices
    int loopStartAbs = static_cast<int>(loopStartNorm * sampleLength);
    int loopEndAbs = static_cast<int>(loopEndNorm * sampleLength);
    loopStartAbs = juce::jlimit(0, sampleLength - 1, loopStartAbs);
    loopEndAbs = juce::jlimit(loopStartAbs + 1, sampleLength, loopEndAbs);

    // Apply zero-crossing snap (cached — only recompute when params change)
    if (snapEnabled && loopEnabled)
    {
        if (loopStartNorm != cachedLoopStartNorm || loopEndNorm != cachedLoopEndNorm || snapEnabled != cachedSnapEnabled)
        {
            cachedLoopStartNorm = loopStartNorm;
            cachedLoopEndNorm = loopEndNorm;
            cachedSnapEnabled = snapEnabled;
            snappedLoopStartAbs = sampleData->findNearestZeroCrossing(loopStartAbs);
            snappedLoopEndAbs = sampleData->findNearestZeroCrossing(loopEndAbs);
            // Ensure snapped end is still after snapped start
            if (snappedLoopEndAbs <= snappedLoopStartAbs)
                snappedLoopEndAbs = loopEndAbs; // fall back to unsnapped
        }
        loopStartAbs = snappedLoopStartAbs;
        loopEndAbs = snappedLoopEndAbs;
    }
    else if (cachedSnapEnabled != snapEnabled)
    {
        cachedSnapEnabled = snapEnabled;
        cachedLoopStartNorm = -1.0f; // invalidate cache
    }
    int loopLength = loopEndAbs - loopStartAbs;

    // Convert crossfade ms to samples, clamp to half loop length
    int crossfadeSamples = static_cast<int>((crossfadeMs / 1000.0f) * static_cast<float>(getSampleRate()));
    crossfadeSamples = juce::jmin(crossfadeSamples, loopLength / 2);

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

        float envelopeValue = adsrEnvelope.getNextSample();

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            int srcChannel = juce::jmin(channel, numChannels - 1);
            float outputSample;

            if (loopEnabled && crossfadeSamples > 0)
            {
                float distanceToEnd = static_cast<float>(loopEndAbs - playbackPosition);

                if (distanceToEnd < crossfadeSamples && distanceToEnd >= 0.0f)
                {
                    // Inside crossfade region — blend outgoing with incoming
                    float fadeProgress = 1.0f - (distanceToEnd / static_cast<float>(crossfadeSamples));
                    float fadeOut = std::sqrt(1.0f - fadeProgress);
                    float fadeIn = std::sqrt(fadeProgress);

                    float outgoing = getInterpolatedSample(sampleBuffer, srcChannel,
                                                           playbackPosition, sampleLength);
                    double incomingPos = static_cast<double>(loopStartAbs) +
                                         (static_cast<double>(crossfadeSamples) - distanceToEnd);
                    float incoming = getInterpolatedSample(sampleBuffer, srcChannel,
                                                           incomingPos, sampleLength);

                    outputSample = outgoing * fadeOut + incoming * fadeIn;
                }
                else
                {
                    outputSample = getInterpolatedSample(sampleBuffer, srcChannel,
                                                          playbackPosition, sampleLength);
                }
            }
            else
            {
                outputSample = getInterpolatedSample(sampleBuffer, srcChannel,
                                                      playbackPosition, sampleLength);
            }

            outputBuffer.addSample(channel, startSample + sample,
                                   outputSample * velocity * envelopeValue);
        }

        playbackPosition += pitchRatio;

        if (loopEnabled)
        {
            if (playbackPosition >= loopEndAbs)
            {
                playbackPosition = loopStartAbs + std::fmod(playbackPosition - loopStartAbs,
                                                             static_cast<double>(loopLength));
                hasCompletedFirstPass = true;
            }
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

void AzmariwVoice::setLoopParameters(std::atomic<float>* playbackMode,
                                      std::atomic<float>* loopStart,
                                      std::atomic<float>* loopEnd,
                                      std::atomic<float>* loopCrossfade,
                                      std::atomic<float>* loopSnapToZero)
{
    playbackModeParam = playbackMode;
    loopStartParam = loopStart;
    loopEndParam = loopEnd;
    loopCrossfadeParam = loopCrossfade;
    loopSnapToZeroParam = loopSnapToZero;
}

void AzmariwVoice::prepareToPlay(double sampleRate)
{
    adsrEnvelope.setSampleRate(sampleRate);
    glideProcessor.prepare(sampleRate);
}
