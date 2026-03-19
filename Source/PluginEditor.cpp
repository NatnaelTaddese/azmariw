#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AzmariwAudioProcessorEditor::AzmariwAudioProcessorEditor(AzmariwAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(1020, 620);

    // Group components
    samplerGroup.setText("Sampler");
    adsrGroup.setText("ADSR Envelope");
    glideGroup.setText("Glide");
    preEqGroup.setText("Pre-EQ");
    distortionGroup.setText("Distortion");
    postEqGroup.setText("Post-EQ");
    masterGroup.setText("Master");

    addAndMakeVisible(samplerGroup);
    addAndMakeVisible(adsrGroup);
    addAndMakeVisible(glideGroup);
    addAndMakeVisible(preEqGroup);
    addAndMakeVisible(distortionGroup);
    addAndMakeVisible(postEqGroup);
    addAndMakeVisible(masterGroup);

    // Drop zone
    dropZoneLabel.setText("Drop WAV/AIFF here", juce::dontSendNotification);
    dropZoneLabel.setJustificationType(juce::Justification::centred);
    dropZoneLabel.setColour(juce::Label::outlineColourId, juce::Colours::grey);
    dropZoneLabel.setColour(juce::Label::backgroundColourId,
                            juce::Colours::black.withAlpha(0.2f));
    addAndMakeVisible(dropZoneLabel);

    // Waveform display
    addAndMakeVisible(waveformDisplay);

    // Sample slot combobox
    slotLabel.setText("Slot:", juce::dontSendNotification);
    slotLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(slotLabel);

    for (int i = 0; i < 8; ++i)
        sampleSlotBox.addItem("Slot " + juce::String(i + 1), i + 1);
    addAndMakeVisible(sampleSlotBox);

    sampleSlotBox.onChange = [this]() { updateDropZoneText(); updateWaveformDisplay(); };

    // Playback mode combobox
    modeLabel.setText("Mode:", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(modeLabel);

    playbackModeBox.addItem("One Shot", 1);
    playbackModeBox.addItem("Loop", 2);
    addAndMakeVisible(playbackModeBox);

    // Root note knob
    setupRotaryKnob(rootNoteKnob, rootNoteLabel, "Root Note");
    rootNoteKnob.setNumDecimalPlacesToDisplay(0);

    // Loop controls
    loopGroup.setText("Loop");
    addAndMakeVisible(loopGroup);
    setupRotaryKnob(loopStartKnob, loopStartLabel, "Start");
    setupRotaryKnob(loopEndKnob, loopEndLabel, "End");
    setupRotaryKnob(loopCrossfadeKnob, loopCrossfadeLabel, "Fade");

    // Snap to zero-crossing toggle
    snapToZeroButton.setButtonText("Zero-X");
    addAndMakeVisible(snapToZeroButton);

    // Update waveform when loop params change
    loopStartKnob.onValueChange = [this]() { updateWaveformDisplay(); };
    loopEndKnob.onValueChange = [this]() { updateWaveformDisplay(); };
    loopCrossfadeKnob.onValueChange = [this]() { updateWaveformDisplay(); };
    snapToZeroButton.onClick = [this]() { updateWaveformDisplay(); };

    // ADSR knobs
    setupRotaryKnob(attackKnob, attackLabel, "Attack");
    setupRotaryKnob(decayKnob, decayLabel, "Decay");
    setupRotaryKnob(sustainKnob, sustainLabel, "Sustain");
    setupRotaryKnob(releaseKnob, releaseLabel, "Release");

    // Glide controls
    glideEnabledButton.setButtonText("Enable");
    addAndMakeVisible(glideEnabledButton);
    setupRotaryKnob(glideTimeKnob, glideTimeLabel, "Time");

    // Pre-EQ knobs
    setupRotaryKnob(preEqFreqKnob, preEqFreqLabel, "Freq");
    setupRotaryKnob(preEqGainKnob, preEqGainLabel, "Gain");
    setupRotaryKnob(preEqQKnob, preEqQLabel, "Q");

    // Distortion type selector
    distortionTypeLabel.setText("Type", juce::dontSendNotification);
    distortionTypeLabel.setJustificationType(juce::Justification::centredRight);
    distortionTypeLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    addAndMakeVisible(distortionTypeLabel);

    distortionTypeBox.addItem("Soft", 1);
    distortionTypeBox.addItem("Tube", 2);
    distortionTypeBox.addItem("Hard", 3);
    distortionTypeBox.addItem("Amp",  4);
    distortionTypeBox.addItem("Fuzz", 5);
    distortionTypeBox.addItem("Fold", 6);
    addAndMakeVisible(distortionTypeBox);

    // Distortion knobs
    setupRotaryKnob(crossoverLowMidKnob, crossoverLowMidLabel, "X Low");
    setupRotaryKnob(crossoverMidHighKnob, crossoverMidHighLabel, "X High");
    setupRotaryKnob(driveLowKnob, driveLowLabel, "Drv Lo");
    setupRotaryKnob(driveMidKnob, driveMidLabel, "Drv Mid");
    setupRotaryKnob(driveHighKnob, driveHighLabel, "Drv Hi");
    setupRotaryKnob(distortionMixKnob, distortionMixLabel, "Mix");

    // Post-EQ knobs
    setupRotaryKnob(postEqFreqKnob, postEqFreqLabel, "Freq");
    setupRotaryKnob(postEqGainKnob, postEqGainLabel, "Gain");
    setupRotaryKnob(postEqQKnob, postEqQLabel, "Q");

    // Master knob
    setupRotaryKnob(masterGainKnob, masterGainLabel, "Gain");

    // Create APVTS attachments (AFTER addAndMakeVisible)
    sampleSlotAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts, ParamIDs::sampleSlot, sampleSlotBox);
    playbackModeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts, ParamIDs::playbackMode, playbackModeBox);
    rootNoteAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::rootNote, rootNoteKnob);

    loopStartAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::loopStart, loopStartKnob);
    loopEndAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::loopEnd, loopEndKnob);
    loopCrossfadeAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::loopCrossfade, loopCrossfadeKnob);
    snapToZeroAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, ParamIDs::loopSnapToZero, snapToZeroButton);

    attackAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::attack, attackKnob);
    decayAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::decay, decayKnob);
    sustainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::sustain, sustainKnob);
    releaseAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::release, releaseKnob);

    glideEnabledAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts, ParamIDs::glideEnabled, glideEnabledButton);
    glideTimeAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::glideTime, glideTimeKnob);

    preEqFreqAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::preEqFrequency, preEqFreqKnob);
    preEqGainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::preEqGain, preEqGainKnob);
    preEqQAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::preEqQ, preEqQKnob);

    crossoverLowMidAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::crossoverLowMid, crossoverLowMidKnob);
    crossoverMidHighAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::crossoverMidHigh, crossoverMidHighKnob);
    driveLowAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::driveLow, driveLowKnob);
    driveMidAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::driveMid, driveMidKnob);
    driveHighAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::driveHigh, driveHighKnob);
    distortionMixAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::distortionMix, distortionMixKnob);
    distortionTypeAttachment = std::make_unique<ComboBoxAttachment>(
        audioProcessor.apvts, ParamIDs::distortionType, distortionTypeBox);

    postEqFreqAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::postEqFrequency, postEqFreqKnob);
    postEqGainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::postEqGain, postEqGainKnob);
    postEqQAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::postEqQ, postEqQKnob);

    masterGainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts, ParamIDs::masterGain, masterGainKnob);

    updateDropZoneText();
    updateWaveformDisplay();
}

AzmariwAudioProcessorEditor::~AzmariwAudioProcessorEditor()
{
}

//==============================================================================
void AzmariwAudioProcessorEditor::setupRotaryKnob(juce::Slider& slider,
                                                    juce::Label& label,
                                                    const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(12.0f)));
    addAndMakeVisible(label);
}

void AzmariwAudioProcessorEditor::updateDropZoneText()
{
    int slot = static_cast<int>(
        audioProcessor.apvts.getRawParameterValue(ParamIDs::sampleSlot)->load());
    auto* sampleData = audioProcessor.getSampleManager().getSampleData(slot);

    if (sampleData != nullptr && sampleData->isLoaded())
        dropZoneLabel.setText(sampleData->getName(), juce::dontSendNotification);
    else
        dropZoneLabel.setText("Drop WAV/AIFF here", juce::dontSendNotification);
}

void AzmariwAudioProcessorEditor::updateWaveformDisplay()
{
    int slot = static_cast<int>(
        audioProcessor.apvts.getRawParameterValue(ParamIDs::sampleSlot)->load());
    auto* sampleData = audioProcessor.getSampleManager().getSampleData(slot);
    waveformDisplay.setSampleData(sampleData);

    float start = audioProcessor.apvts.getRawParameterValue(ParamIDs::loopStart)->load();
    float end = audioProcessor.apvts.getRawParameterValue(ParamIDs::loopEnd)->load();
    float fade = audioProcessor.apvts.getRawParameterValue(ParamIDs::loopCrossfade)->load();
    bool snap = audioProcessor.apvts.getRawParameterValue(ParamIDs::loopSnapToZero)->load() >= 0.5f;
    waveformDisplay.setLoopParameters(start, end, fade);
    waveformDisplay.setSnapEnabled(snap);

    // Compute snapped positions for visual feedback
    if (snap && sampleData != nullptr && sampleData->isLoaded())
    {
        int numSamples = sampleData->getBuffer().getNumSamples();
        int startAbs = static_cast<int>(start * numSamples);
        int endAbs = static_cast<int>(end * numSamples);
        int snappedStartAbs = sampleData->findNearestZeroCrossing(startAbs);
        int snappedEndAbs = sampleData->findNearestZeroCrossing(endAbs);
        float snappedStartNorm = static_cast<float>(snappedStartAbs) / static_cast<float>(numSamples);
        float snappedEndNorm = static_cast<float>(snappedEndAbs) / static_cast<float>(numSamples);
        waveformDisplay.setSnappedPositions(snappedStartNorm, snappedEndNorm);
    }
    else
    {
        waveformDisplay.setSnappedPositions(start, end);
    }
}

//==============================================================================
void AzmariwAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(22.0f).withStyle("Bold")));
    g.drawText("AZMARIW", getLocalBounds().removeFromTop(40),
               juce::Justification::centred, true);

    // Drop zone highlight when dragging
    if (dragOver)
    {
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.fillRect(dropZoneLabel.getBounds().expanded(2));
    }
}

//==============================================================================
void AzmariwAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(40); // title area

    // Waveform display row
    auto waveformArea = bounds.removeFromTop(100).reduced(8, 4);
    waveformDisplay.setBounds(waveformArea);

    auto topRow = bounds.removeFromTop(220);
    auto bottomRow = bounds.reduced(0, 4);

    // Top row: Sampler | Loop | ADSR | Glide
    auto samplerArea = topRow.removeFromLeft(240).reduced(4);
    auto loopArea = topRow.removeFromLeft(200).reduced(4);
    auto adsrArea = topRow.removeFromLeft(340).reduced(4);
    auto glideArea = topRow.reduced(4);

    // Bottom row: Pre-EQ | Distortion | Post-EQ | Master
    auto preEqArea = bottomRow.removeFromLeft(190).reduced(4);
    auto distortionArea = bottomRow.removeFromLeft(340).reduced(4);
    auto postEqArea = bottomRow.removeFromLeft(190).reduced(4);
    auto masterArea = bottomRow.reduced(4);

    // Set group bounds
    samplerGroup.setBounds(samplerArea);
    adsrGroup.setBounds(adsrArea);
    glideGroup.setBounds(glideArea);
    preEqGroup.setBounds(preEqArea);
    distortionGroup.setBounds(distortionArea);
    postEqGroup.setBounds(postEqArea);
    masterGroup.setBounds(masterArea);

    // --- Sampler section layout ---
    auto samplerInner = samplerArea.reduced(10).withTrimmedTop(15);
    dropZoneLabel.setBounds(samplerInner.removeFromTop(40));
    samplerInner.removeFromTop(8);

    auto slotRow = samplerInner.removeFromTop(28);
    slotLabel.setBounds(slotRow.removeFromLeft(40));
    sampleSlotBox.setBounds(slotRow.reduced(2, 0));
    samplerInner.removeFromTop(6);

    auto modeRow = samplerInner.removeFromTop(28);
    modeLabel.setBounds(modeRow.removeFromLeft(40));
    playbackModeBox.setBounds(modeRow.reduced(2, 0));
    samplerInner.removeFromTop(6);

    auto rootArea = samplerInner;
    auto knobSize = juce::jmin(rootArea.getWidth(), rootArea.getHeight() - 18);
    auto rootKnobArea = rootArea.withSizeKeepingCentre(knobSize, knobSize);
    rootNoteKnob.setBounds(rootKnobArea);
    rootNoteLabel.setBounds(rootKnobArea.getX(), rootKnobArea.getBottom(),
                            rootKnobArea.getWidth(), 18);

    // --- Loop section layout ---
    loopGroup.setBounds(loopArea);
    auto loopInner = loopArea.reduced(10).withTrimmedTop(15);

    // Snap toggle at top of loop section
    snapToZeroButton.setBounds(loopInner.removeFromTop(24));
    loopInner.removeFromTop(4);

    int loopKnobWidth = loopInner.getWidth() / 3;
    int loopLabelH = 18;

    auto loopLabelRow = loopInner.removeFromTop(loopLabelH);
    loopStartLabel.setBounds(loopLabelRow.removeFromLeft(loopKnobWidth));
    loopEndLabel.setBounds(loopLabelRow.removeFromLeft(loopKnobWidth));
    loopCrossfadeLabel.setBounds(loopLabelRow);

    auto loopKnobRow = loopInner;
    loopStartKnob.setBounds(loopKnobRow.removeFromLeft(loopKnobWidth).reduced(2));
    loopEndKnob.setBounds(loopKnobRow.removeFromLeft(loopKnobWidth).reduced(2));
    loopCrossfadeKnob.setBounds(loopKnobRow.reduced(2));

    // --- ADSR section layout ---
    auto adsrInner = adsrArea.reduced(10).withTrimmedTop(15);
    int adsrKnobWidth = adsrInner.getWidth() / 4;
    int adsrLabelH = 18;

    auto adsrLabelRow = adsrInner.removeFromTop(adsrLabelH);
    attackLabel.setBounds(adsrLabelRow.removeFromLeft(adsrKnobWidth));
    decayLabel.setBounds(adsrLabelRow.removeFromLeft(adsrKnobWidth));
    sustainLabel.setBounds(adsrLabelRow.removeFromLeft(adsrKnobWidth));
    releaseLabel.setBounds(adsrLabelRow);

    auto adsrKnobRow = adsrInner;
    attackKnob.setBounds(adsrKnobRow.removeFromLeft(adsrKnobWidth).reduced(4));
    decayKnob.setBounds(adsrKnobRow.removeFromLeft(adsrKnobWidth).reduced(4));
    sustainKnob.setBounds(adsrKnobRow.removeFromLeft(adsrKnobWidth).reduced(4));
    releaseKnob.setBounds(adsrKnobRow.reduced(4));

    // --- Glide section layout ---
    auto glideInner = glideArea.reduced(10).withTrimmedTop(15);
    glideEnabledButton.setBounds(glideInner.removeFromTop(28));
    glideInner.removeFromTop(4);
    glideTimeLabel.setBounds(glideInner.removeFromTop(adsrLabelH));
    glideTimeKnob.setBounds(glideInner.reduced(10));

    // --- Pre-EQ section layout ---
    auto preEqInner = preEqArea.reduced(10).withTrimmedTop(15);
    int eqKnobWidth = preEqInner.getWidth() / 3;

    auto preEqLabelRow = preEqInner.removeFromTop(adsrLabelH);
    preEqFreqLabel.setBounds(preEqLabelRow.removeFromLeft(eqKnobWidth));
    preEqGainLabel.setBounds(preEqLabelRow.removeFromLeft(eqKnobWidth));
    preEqQLabel.setBounds(preEqLabelRow);

    auto preEqKnobRow = preEqInner;
    preEqFreqKnob.setBounds(preEqKnobRow.removeFromLeft(eqKnobWidth).reduced(2));
    preEqGainKnob.setBounds(preEqKnobRow.removeFromLeft(eqKnobWidth).reduced(2));
    preEqQKnob.setBounds(preEqKnobRow.reduced(2));

    // --- Distortion section layout ---
    auto distInner = distortionArea.reduced(10).withTrimmedTop(15);

    // Type selector row at top
    auto distTypeRow = distInner.removeFromTop(22);
    distortionTypeLabel.setBounds(distTypeRow.removeFromLeft(35));
    distortionTypeBox.setBounds(distTypeRow.reduced(2, 0));
    distInner.removeFromTop(4);

    int distKnobWidth = distInner.getWidth() / 6;

    auto distLabelRow = distInner.removeFromTop(adsrLabelH);
    crossoverLowMidLabel.setBounds(distLabelRow.removeFromLeft(distKnobWidth));
    crossoverMidHighLabel.setBounds(distLabelRow.removeFromLeft(distKnobWidth));
    driveLowLabel.setBounds(distLabelRow.removeFromLeft(distKnobWidth));
    driveMidLabel.setBounds(distLabelRow.removeFromLeft(distKnobWidth));
    driveHighLabel.setBounds(distLabelRow.removeFromLeft(distKnobWidth));
    distortionMixLabel.setBounds(distLabelRow);

    auto distKnobRow = distInner;
    crossoverLowMidKnob.setBounds(distKnobRow.removeFromLeft(distKnobWidth).reduced(2));
    crossoverMidHighKnob.setBounds(distKnobRow.removeFromLeft(distKnobWidth).reduced(2));
    driveLowKnob.setBounds(distKnobRow.removeFromLeft(distKnobWidth).reduced(2));
    driveMidKnob.setBounds(distKnobRow.removeFromLeft(distKnobWidth).reduced(2));
    driveHighKnob.setBounds(distKnobRow.removeFromLeft(distKnobWidth).reduced(2));
    distortionMixKnob.setBounds(distKnobRow.reduced(2));

    // --- Post-EQ section layout ---
    auto postEqInner = postEqArea.reduced(10).withTrimmedTop(15);
    int postEqKnobWidth = postEqInner.getWidth() / 3;

    auto postEqLabelRow = postEqInner.removeFromTop(adsrLabelH);
    postEqFreqLabel.setBounds(postEqLabelRow.removeFromLeft(postEqKnobWidth));
    postEqGainLabel.setBounds(postEqLabelRow.removeFromLeft(postEqKnobWidth));
    postEqQLabel.setBounds(postEqLabelRow);

    auto postEqKnobRow = postEqInner;
    postEqFreqKnob.setBounds(postEqKnobRow.removeFromLeft(postEqKnobWidth).reduced(2));
    postEqGainKnob.setBounds(postEqKnobRow.removeFromLeft(postEqKnobWidth).reduced(2));
    postEqQKnob.setBounds(postEqKnobRow.reduced(2));

    // --- Master section layout ---
    auto masterInner = masterArea.reduced(10).withTrimmedTop(15);
    masterGainLabel.setBounds(masterInner.removeFromTop(adsrLabelH));
    masterGainKnob.setBounds(masterInner.reduced(4));
}

//==============================================================================
// FileDragAndDropTarget
//==============================================================================
bool AzmariwAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".wav") || file.endsWithIgnoreCase(".aiff")
            || file.endsWithIgnoreCase(".aif"))
            return true;
    }
    return false;
}

void AzmariwAudioProcessorEditor::filesDropped(const juce::StringArray& files,
                                                int /*x*/, int /*y*/)
{
    dragOver = false;
    repaint();

    for (const auto& filePath : files)
    {
        juce::File file(filePath);
        if (file.hasFileExtension("wav;aiff;aif"))
        {
            int slot = static_cast<int>(
                audioProcessor.apvts.getRawParameterValue(ParamIDs::sampleSlot)->load());
            audioProcessor.loadSampleFromFile(slot, file);
            updateDropZoneText();
            updateWaveformDisplay();
            break;
        }
    }
}

void AzmariwAudioProcessorEditor::fileDragEnter(const juce::StringArray& /*files*/,
                                                  int /*x*/, int /*y*/)
{
    dragOver = true;
    repaint();
}

void AzmariwAudioProcessorEditor::fileDragExit(const juce::StringArray& /*files*/)
{
    dragOver = false;
    repaint();
}
