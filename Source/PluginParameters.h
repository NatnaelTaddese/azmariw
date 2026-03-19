#pragma once

#include <JuceHeader.h>

namespace ParamIDs
{
    // Sampler
    inline constexpr auto sampleSlot    = "sampleSlot";
    inline constexpr auto playbackMode  = "playbackMode";
    inline constexpr auto loopStart     = "loopStart";
    inline constexpr auto loopEnd       = "loopEnd";
    inline constexpr auto loopCrossfade = "loopCrossfade";
    inline constexpr auto rootNote      = "rootNote";

    // ADSR Envelope
    inline constexpr auto attack  = "attack";
    inline constexpr auto decay   = "decay";
    inline constexpr auto sustain = "sustain";
    inline constexpr auto release = "release";

    // Glide
    inline constexpr auto glideEnabled = "glideEnabled";
    inline constexpr auto glideTime    = "glideTime";

    // Pre-EQ
    inline constexpr auto preEqFrequency = "preEqFrequency";
    inline constexpr auto preEqGain      = "preEqGain";
    inline constexpr auto preEqQ         = "preEqQ";

    // Distortion
    inline constexpr auto crossoverLowMid  = "crossoverLowMid";
    inline constexpr auto crossoverMidHigh = "crossoverMidHigh";
    inline constexpr auto driveLow         = "driveLow";
    inline constexpr auto driveMid         = "driveMid";
    inline constexpr auto driveHigh        = "driveHigh";
    inline constexpr auto distortionMix    = "distortionMix";

    // Post-EQ
    inline constexpr auto postEqFrequency = "postEqFrequency";
    inline constexpr auto postEqGain      = "postEqGain";
    inline constexpr auto postEqQ         = "postEqQ";

    // Master
    inline constexpr auto masterGain = "masterGain";
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Sampler parameters
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ParamIDs::sampleSlot, 1}, "Sample Slot",
        0, 7, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamIDs::playbackMode, 1}, "Playback Mode",
        juce::StringArray{"One Shot", "Loop"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::loopStart, 1}, "Loop Start",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::loopEnd, 1}, "Loop End",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::loopCrossfade, 1}, "Loop Crossfade",
        juce::NormalisableRange<float>(0.0f, 500.0f, 0.1f, 0.5f), 20.0f));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ParamIDs::rootNote, 1}, "Root Note",
        0, 127, 60));

    // ADSR Envelope
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::attack, 1}, "Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.3f), 0.01f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::decay, 1}, "Decay",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.3f), 0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::sustain, 1}, "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::release, 1}, "Release",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f), 0.1f));

    // Glide
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamIDs::glideEnabled, 1}, "Glide Enabled", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::glideTime, 1}, "Glide Time",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.5f), 0.1f));

    // Pre-EQ
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::preEqFrequency, 1}, "Pre EQ Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::preEqGain, 1}, "Pre EQ Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::preEqQ, 1}, "Pre EQ Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 1.0f));

    // Distortion
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::crossoverLowMid, 1}, "Crossover Low-Mid",
        juce::NormalisableRange<float>(80.0f, 2000.0f, 1.0f, 0.3f), 250.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::crossoverMidHigh, 1}, "Crossover Mid-High",
        juce::NormalisableRange<float>(2000.0f, 16000.0f, 1.0f, 0.3f), 4000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::driveLow, 1}, "Drive Low",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::driveMid, 1}, "Drive Mid",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::driveHigh, 1}, "Drive High",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::distortionMix, 1}, "Distortion Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // Post-EQ
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::postEqFrequency, 1}, "Post EQ Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::postEqGain, 1}, "Post EQ Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::postEqQ, 1}, "Post EQ Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 1.0f));

    // Master
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamIDs::masterGain, 1}, "Master Gain",
        juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f));

    return layout;
}
