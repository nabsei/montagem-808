#include "E808Processor.h"
#include "PluginEditor.h"

E808Processor::E808Processor()
    : AudioProcessor(BusesProperties()
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout E808Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "amount", "Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    return { params.begin(), params.end() };
}

void E808Processor::prepareToPlay(double sampleRate, int)
{
    lastSampleRate = sampleRate;

    attackSamplesTotal = juce::jmax(1, (int)(sampleRate * 0.005));
    attackSamplesDone = 0;
    inAttack = false;
    ampEnvelope = 0.0f;
    phase = 0.0;
    noteIsOn = false;

    amountSmoothed.reset(sampleRate, 0.03);
    amountSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("amount"));
}

void E808Processor::startNote(int midiNote, float velocity)
{
    targetFreq = 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);

    const float amount01 = amountSmoothed.getCurrentValue();
    const float glideSemitones = juce::jmap(amount01, 0.0f, 1.0f, 3.0f, 9.0f);
    glideStartFreq = targetFreq * std::pow(2.0, glideSemitones / 12.0);
    currentFreq = glideStartFreq;

    const float glideTime = juce::jmap(amount01, 0.0f, 1.0f, 0.12f, 0.30f);
    glideSamplesTotal = juce::jmax(1, (int)(lastSampleRate * glideTime));
    glideSamplesDone = 0;

    phase = 0.0;
    ampEnvelope = 0.0f;
    inAttack = true;
    attackSamplesDone = 0;

    velocityGain = juce::jlimit(0.2f, 1.0f, velocity);
    noteIsOn = true;
    lastPlayedNote.store(midiNote, std::memory_order_relaxed);
}

void E808Processor::stopNote()
{
    noteIsOn = false;
    lastPlayedNote.store(-1, std::memory_order_relaxed);
}

void E808Processor::handleMidiEvent(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
        startNote(message.getNoteNumber(), message.getFloatVelocity());
    else if (message.isNoteOff())
        stopNote();
}

void E808Processor::renderBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (numSamples <= 0)
        return;

    const float amount01 = amountSmoothed.skip(numSamples);
    const float decayTau = juce::jmap(amount01, 0.0f, 1.0f, 0.2f, 0.7f);
    const float sineTriMix = juce::jmap(amount01, 0.0f, 1.0f, 0.8f, 0.65f);
    const float decayCoef = std::exp(-1.0f / (float)(lastSampleRate * decayTau));

    auto* left = buffer.getWritePointer(0, startSample);
    auto* right = buffer.getWritePointer(1, startSample);

    for (int i = 0; i < numSamples; ++i)
    {
        if (inAttack)
        {
            ampEnvelope = (float)attackSamplesDone / (float)attackSamplesTotal;
            ++attackSamplesDone;
            if (attackSamplesDone >= attackSamplesTotal)
            {
                inAttack = false;
                ampEnvelope = 1.0f;
            }
        }
        else
        {
            ampEnvelope *= decayCoef;
        }

        if (glideSamplesDone < glideSamplesTotal)
        {
            const double t = (double)glideSamplesDone / (double)glideSamplesTotal;
            currentFreq = glideStartFreq * std::pow(targetFreq / glideStartFreq, t);
            ++glideSamplesDone;
        }
        else
        {
            currentFreq = targetFreq;
        }

        phase += juce::MathConstants<double>::twoPi * currentFreq / lastSampleRate;
        if (phase > juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        const float sineVal = (float)std::sin(phase);
        const double normPhase = phase / juce::MathConstants<double>::twoPi;
        const float triVal = (float)(2.0 * std::abs(2.0 * normPhase - 1.0) - 1.0);
        const float osc = sineTriMix * sineVal + (1.0f - sineTriMix) * triVal;

        const float sample = osc * ampEnvelope * velocityGain * 0.8f;
        left[i] = sample;
        right[i] = sample;
    }
}

void E808Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    amountSmoothed.setTargetValue(*apvts.getRawParameterValue("amount"));

    int samplePos = 0;
    for (const auto metadata : midiMessages)
    {
        const int eventSample = metadata.samplePosition;
        if (eventSample > samplePos)
        {
            renderBlock(buffer, samplePos, eventSample - samplePos);
            samplePos = eventSample;
        }
        handleMidiEvent(metadata.getMessage());
    }

    if (samplePos < buffer.getNumSamples())
        renderBlock(buffer, samplePos, buffer.getNumSamples() - samplePos);
}

juce::AudioProcessorEditor* E808Processor::createEditor()
{
    return new E808Editor(*this);
}

void E808Processor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
}

void E808Processor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
