#include "SampleManager.h"

SampleManager::SampleManager()
{
    formatManager.registerBasicFormats();

    for (auto& slot : slots)
        slot = std::make_unique<SampleData>();
}

void SampleManager::loadPresetSamples(const juce::File& sampleDirectory)
{
    if (!sampleDirectory.isDirectory())
        return;

    auto files = sampleDirectory.findChildFiles(
        juce::File::findFiles, false, "*.wav;*.aiff;*.aif");

    files.sort();

    int slotIndex = 0;
    for (const auto& file : files)
    {
        if (slotIndex >= maxSlots)
            break;

        const juce::SpinLock::ScopedLockType lock(sampleLock);
        slots[slotIndex]->loadFromFile(file, formatManager);
        slotIndex++;
    }
}

bool SampleManager::loadUserSample(int slot, const juce::File& file)
{
    if (slot < 0 || slot >= maxSlots)
        return false;

    auto newSample = std::make_unique<SampleData>();
    if (!newSample->loadFromFile(file, formatManager))
        return false;

    const juce::SpinLock::ScopedLockType lock(sampleLock);
    slots[slot] = std::move(newSample);
    return true;
}

SampleData* SampleManager::getSampleData(int slot)
{
    if (slot < 0 || slot >= maxSlots)
        return nullptr;

    return slots[slot].get();
}

const SampleData* SampleManager::getSampleData(int slot) const
{
    if (slot < 0 || slot >= maxSlots)
        return nullptr;

    return slots[slot].get();
}

void SampleManager::saveState(juce::ValueTree& state) const
{
    juce::ValueTree samplesTree("Samples");

    for (int i = 0; i < maxSlots; ++i)
    {
        if (slots[i] != nullptr && slots[i]->isLoaded())
        {
            juce::ValueTree slotTree("Slot");
            slotTree.setProperty("index", i, nullptr);
            slotTree.setProperty("filePath", slots[i]->getFilePath(), nullptr);
            slotTree.setProperty("rootNote", slots[i]->getRootNote(), nullptr);
            slotTree.setProperty("loopEnabled", slots[i]->isLoopEnabled(), nullptr);
            slotTree.setProperty("loopStart", slots[i]->getLoopStart(), nullptr);
            slotTree.setProperty("loopEnd", slots[i]->getLoopEnd(), nullptr);
            samplesTree.addChild(slotTree, -1, nullptr);
        }
    }

    state.addChild(samplesTree, -1, nullptr);
}

void SampleManager::loadState(const juce::ValueTree& state)
{
    auto samplesTree = state.getChildWithName("Samples");
    if (!samplesTree.isValid())
        return;

    for (int i = 0; i < samplesTree.getNumChildren(); ++i)
    {
        auto slotTree = samplesTree.getChild(i);
        int slotIndex = slotTree.getProperty("index", -1);

        if (slotIndex < 0 || slotIndex >= maxSlots)
            continue;

        juce::String path = slotTree.getProperty("filePath", "");
        juce::File file(path);

        if (file.existsAsFile())
        {
            const juce::SpinLock::ScopedLockType lock(sampleLock);
            slots[slotIndex]->loadFromFile(file, formatManager);
            slots[slotIndex]->setRootNote(slotTree.getProperty("rootNote", 60));
            slots[slotIndex]->setLoopEnabled(slotTree.getProperty("loopEnabled", false));
            slots[slotIndex]->setLoopPoints(
                slotTree.getProperty("loopStart", 0),
                slotTree.getProperty("loopEnd", slots[slotIndex]->getBuffer().getNumSamples()));
        }
    }
}
