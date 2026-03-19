#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
    startTimerHz(30);
}

void WaveformDisplay::setSampleData(const SampleData* sample)
{
    if (currentSample != sample)
    {
        currentSample = sample;
        pathDirty = true;
        repaint();
    }
}

void WaveformDisplay::setLoopParameters(float start, float end, float fade)
{
    if (loopStart != start || loopEnd != end || crossfadeMs != fade)
    {
        loopStart = start;
        loopEnd = end;
        crossfadeMs = fade;
        repaint();
    }
}

void WaveformDisplay::setSnapEnabled(bool enabled)
{
    if (snapEnabled != enabled)
    {
        snapEnabled = enabled;
        repaint();
    }
}

void WaveformDisplay::setSnappedPositions(float snappedStartNorm, float snappedEndNorm)
{
    if (snappedStart != snappedStartNorm || snappedEnd != snappedEndNorm)
    {
        snappedStart = snappedStartNorm;
        snappedEnd = snappedEndNorm;
        repaint();
    }
}

void WaveformDisplay::timerCallback()
{
    // Periodically check if we need to refresh
    if (pathDirty && getWidth() > 0)
    {
        rebuildWaveformPath();
        pathDirty = false;
        repaint();
    }
}

void WaveformDisplay::resized()
{
    pathDirty = true;
}

void WaveformDisplay::rebuildWaveformPath()
{
    waveformPeaks.clear();
    waveformPath.clear();

    if (currentSample == nullptr || !currentSample->isLoaded())
        return;

    int width = getWidth();
    if (width <= 0)
        return;

    cachedWidth = width;
    const auto& buffer = currentSample->getBuffer();
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    if (numSamples == 0)
        return;

    // Build peak data — store min/max per pixel column
    waveformPeaks.resize(static_cast<size_t>(width * 2));

    double samplesPerPixel = static_cast<double>(numSamples) / width;

    for (int x = 0; x < width; ++x)
    {
        int startIdx = static_cast<int>(x * samplesPerPixel);
        int endIdx = static_cast<int>((x + 1) * samplesPerPixel);
        endIdx = juce::jmin(endIdx, numSamples);

        float minVal = 0.0f;
        float maxVal = 0.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int i = startIdx; i < endIdx; ++i)
            {
                float s = data[i];
                if (s < minVal) minVal = s;
                if (s > maxVal) maxVal = s;
            }
        }

        waveformPeaks[static_cast<size_t>(x * 2)] = minVal;
        waveformPeaks[static_cast<size_t>(x * 2 + 1)] = maxVal;
    }
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(bounds, 4.0f);

    if (currentSample == nullptr || !currentSample->isLoaded() || waveformPeaks.empty())
    {
        // Empty state
        g.setColour(juce::Colours::grey.withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions(13.0f)));
        g.drawText("No sample loaded", bounds, juce::Justification::centred);
        return;
    }

    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float centreY = bounds.getY() + height * 0.5f;
    float halfHeight = height * 0.45f; // leave small margin

    // Draw loop region highlight
    float loopStartX = bounds.getX() + loopStart * width;
    float loopEndX = bounds.getX() + loopEnd * width;

    // Loop region background
    g.setColour(juce::Colour(0xff2d5a7b).withAlpha(0.3f));
    g.fillRect(loopStartX, bounds.getY(), loopEndX - loopStartX, height);

    // Crossfade region overlay
    if (crossfadeMs > 0.0f && currentSample->getSourceSampleRate() > 0)
    {
        int numSamples = currentSample->getBuffer().getNumSamples();
        float crossfadeFraction = (crossfadeMs / 1000.0f) *
                                   static_cast<float>(currentSample->getSourceSampleRate()) /
                                   static_cast<float>(numSamples);
        float crossfadeWidth = crossfadeFraction * width;

        // Clamp crossfade to half loop length
        float loopWidth = loopEndX - loopStartX;
        crossfadeWidth = juce::jmin(crossfadeWidth, loopWidth * 0.5f);

        if (crossfadeWidth > 1.0f)
        {
            // Fade-out region (before loop end)
            float fadeOutStartX = loopEndX - crossfadeWidth;
            g.setColour(juce::Colour(0xffcc6633).withAlpha(0.25f));
            g.fillRect(fadeOutStartX, bounds.getY(), crossfadeWidth, height);

            // Fade-in region (after loop start)
            g.setColour(juce::Colour(0xff33cc66).withAlpha(0.25f));
            g.fillRect(loopStartX, bounds.getY(), crossfadeWidth, height);
        }
    }

    // Draw waveform
    int peakCount = static_cast<int>(waveformPeaks.size()) / 2;
    if (peakCount != cachedWidth)
    {
        pathDirty = true;
        return;
    }

    // Waveform fill
    juce::Path fillPath;
    fillPath.startNewSubPath(bounds.getX(), centreY);

    for (int x = 0; x < peakCount; ++x)
    {
        float maxVal = waveformPeaks[static_cast<size_t>(x * 2 + 1)];
        float drawX = bounds.getX() + static_cast<float>(x);
        fillPath.lineTo(drawX, centreY - maxVal * halfHeight);
    }

    for (int x = peakCount - 1; x >= 0; --x)
    {
        float minVal = waveformPeaks[static_cast<size_t>(x * 2)];
        float drawX = bounds.getX() + static_cast<float>(x);
        fillPath.lineTo(drawX, centreY - minVal * halfHeight);
    }

    fillPath.closeSubPath();

    // Filled waveform with gradient
    g.setColour(juce::Colour(0xffdd8833).withAlpha(0.7f));
    g.fillPath(fillPath);

    // Waveform outline
    g.setColour(juce::Colour(0xffee9944));
    for (int x = 0; x < peakCount; ++x)
    {
        float minVal = waveformPeaks[static_cast<size_t>(x * 2)];
        float maxVal = waveformPeaks[static_cast<size_t>(x * 2 + 1)];
        float drawX = bounds.getX() + static_cast<float>(x);
        g.drawVerticalLine(static_cast<int>(drawX),
                           centreY - maxVal * halfHeight,
                           centreY - minVal * halfHeight);
    }

    // Centre line
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawHorizontalLine(static_cast<int>(centreY), bounds.getX(), bounds.getRight());

    // Loop boundary lines
    g.setColour(juce::Colour(0xff4da6ff));
    g.drawVerticalLine(static_cast<int>(loopStartX), bounds.getY(), bounds.getBottom());
    g.drawVerticalLine(static_cast<int>(loopEndX), bounds.getY(), bounds.getBottom());

    // Loop start/end markers (small triangles)
    float markerSize = 6.0f;

    // Start marker (top)
    juce::Path startMarker;
    startMarker.addTriangle(loopStartX, bounds.getY(),
                            loopStartX + markerSize, bounds.getY(),
                            loopStartX, bounds.getY() + markerSize);
    g.fillPath(startMarker);

    // End marker (top)
    juce::Path endMarker;
    endMarker.addTriangle(loopEndX, bounds.getY(),
                          loopEndX - markerSize, bounds.getY(),
                          loopEndX, bounds.getY() + markerSize);
    g.fillPath(endMarker);

    // Draw snapped position indicators (green dashed lines)
    if (snapEnabled)
    {
        float snappedStartX = bounds.getX() + snappedStart * width;
        float snappedEndX = bounds.getX() + snappedEnd * width;

        g.setColour(juce::Colour(0xff33cc66));

        // Snapped start line
        float dashLen = 4.0f;
        for (float y = bounds.getY(); y < bounds.getBottom(); y += dashLen * 2)
        {
            float segEnd = juce::jmin(y + dashLen, bounds.getBottom());
            g.drawVerticalLine(static_cast<int>(snappedStartX), y, segEnd);
            g.drawVerticalLine(static_cast<int>(snappedEndX), y, segEnd);
        }

        // Small diamond markers at snapped positions
        float diamondSize = 5.0f;
        float midY = centreY;

        juce::Path startDiamond;
        startDiamond.addTriangle(snappedStartX - diamondSize, midY,
                                  snappedStartX, midY - diamondSize,
                                  snappedStartX, midY + diamondSize);
        g.fillPath(startDiamond);

        juce::Path endDiamond;
        endDiamond.addTriangle(snappedEndX + diamondSize, midY,
                                snappedEndX, midY - diamondSize,
                                snappedEndX, midY + diamondSize);
        g.fillPath(endDiamond);
    }

    // Border
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
}
