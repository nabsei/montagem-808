#pragma once
#include "E808LookAndFeel.h"
#include "E808Processor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class E808Editor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit E808Editor(E808Processor&);
    ~E808Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    E808Processor& processorRef;
    E808LookAndFeel lookAndFeel;

    juce::Slider amountSlider;
    juce::Label amountValueLabel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label footerLabel;
    juce::Label brandLabel;
    juce::Label noteLabel;
    juce::MidiKeyboardComponent keyboardComponent;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;

    void updateValueLabel();
    void timerCallback() override;
    static juce::String midiNoteToName(int noteNumber);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(E808Editor)
};
