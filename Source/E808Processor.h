#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// One-knob 808 slide synth, the fourth piece of the Montagem chain and the
// one that comes FIRST in the signal path: generate the 808 here, then run
// it through Finisher (drive/loudness), Widener (space), and Punch
// (transient) to finish it. Monophonic, with a pitch glide from an octave
// above the played note down to the note itself -- the classic "808 slide"
// shape, ported from the same algorithm used to synthesize the earliest
// demo material for this whole project (gen_808.py).
//
// NOTE: this is the public / portfolio version. The tuned constants
// (glide range, decay curve, oscillator blend) used in the shipped build
// are simplified/omitted here -- this file demonstrates the JUCE
// architecture (including MIDI/voice handling), not the production
// calibration.
class E808Processor : public juce::AudioProcessor
{
public:
    E808Processor();
    ~E808Processor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Explicitly stereo-output only. renderBlock() writes to channels 0 and 1
    // unconditionally (it's always generating a mono voice into a stereo
    // bus) -- without this override, a host/validator can request a mono
    // output layout and the hardcoded channel-1 write becomes an
    // out-of-bounds access. Found via `auval`'s 1-channel render test, which
    // segfaulted before this was added.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Montagem 808"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // Exposed for the UI: -1 when no note is active, otherwise the last
    // MIDI note number played (so the editor can show what's sounding).
    std::atomic<int> lastPlayedNote { -1 };

    // Lives on the processor (not the editor) so it keeps working/persists
    // correctly across the editor being closed and reopened. The editor's
    // on-screen keyboard component reads/writes this; processBlock() merges
    // it into the real MIDI stream every block. JUCE's standalone wrapper
    // does not provide its own virtual keyboard, so without this there is
    // no way to play/test the synth without a physical MIDI controller.
    juce::MidiKeyboardState keyboardState;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void renderBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void handleMidiEvent(const juce::MidiMessage& message);
    void startNote(int midiNote, float velocity);
    void stopNote();

    double lastSampleRate = 44100.0;

    // Voice state -- monophonic, single active note at a time.
    bool noteIsOn = false;
    double phase = 0.0;
    double currentFreq = 110.0;
    double targetFreq = 110.0;
    double glideStartFreq = 110.0;
    int glideSamplesTotal = 0;
    int glideSamplesDone = 0;

    float velocityGain = 1.0f;
    float ampEnvelope = 0.0f;
    int attackSamplesTotal = 0;
    int attackSamplesDone = 0;
    bool inAttack = false;

    juce::SmoothedValue<float> amountSmoothed;
};
