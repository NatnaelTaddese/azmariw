#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginParameters.h"

class AzmariwAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     public juce::FileDragAndDropTarget
{
public:
    AzmariwAudioProcessorEditor(AzmariwAudioProcessor&);
    ~AzmariwAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;

private:
    void setupRotaryKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void updateDropZoneText();

    AzmariwAudioProcessor& audioProcessor;
    bool dragOver = false;

    // Group components
    juce::GroupComponent samplerGroup, adsrGroup, glideGroup;
    juce::GroupComponent preEqGroup, distortionGroup, postEqGroup, masterGroup;

    // Sampler section
    juce::ComboBox sampleSlotBox;
    juce::ComboBox playbackModeBox;
    juce::Slider rootNoteKnob;
    juce::Label rootNoteLabel;
    juce::Label dropZoneLabel;
    juce::Label slotLabel, modeLabel;

    // ADSR section
    juce::Slider attackKnob, decayKnob, sustainKnob, releaseKnob;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    // Glide section
    juce::ToggleButton glideEnabledButton;
    juce::Slider glideTimeKnob;
    juce::Label glideTimeLabel;

    // Pre-EQ section
    juce::Slider preEqFreqKnob, preEqGainKnob, preEqQKnob;
    juce::Label preEqFreqLabel, preEqGainLabel, preEqQLabel;

    // Distortion section
    juce::Slider crossoverLowMidKnob, crossoverMidHighKnob;
    juce::Slider driveLowKnob, driveMidKnob, driveHighKnob;
    juce::Slider distortionMixKnob;
    juce::Label crossoverLowMidLabel, crossoverMidHighLabel;
    juce::Label driveLowLabel, driveMidLabel, driveHighLabel;
    juce::Label distortionMixLabel;

    // Post-EQ section
    juce::Slider postEqFreqKnob, postEqGainKnob, postEqQKnob;
    juce::Label postEqFreqLabel, postEqGainLabel, postEqQLabel;

    // Master section
    juce::Slider masterGainKnob;
    juce::Label masterGainLabel;

    // APVTS Attachments — declared AFTER widgets for correct destruction order
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ComboBoxAttachment> sampleSlotAttachment;
    std::unique_ptr<ComboBoxAttachment> playbackModeAttachment;
    std::unique_ptr<SliderAttachment> rootNoteAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment, decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment, releaseAttachment;

    std::unique_ptr<ButtonAttachment> glideEnabledAttachment;
    std::unique_ptr<SliderAttachment> glideTimeAttachment;

    std::unique_ptr<SliderAttachment> preEqFreqAttachment, preEqGainAttachment, preEqQAttachment;

    std::unique_ptr<SliderAttachment> crossoverLowMidAttachment, crossoverMidHighAttachment;
    std::unique_ptr<SliderAttachment> driveLowAttachment, driveMidAttachment, driveHighAttachment;
    std::unique_ptr<SliderAttachment> distortionMixAttachment;

    std::unique_ptr<SliderAttachment> postEqFreqAttachment, postEqGainAttachment, postEqQAttachment;

    std::unique_ptr<SliderAttachment> masterGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AzmariwAudioProcessorEditor)
};
