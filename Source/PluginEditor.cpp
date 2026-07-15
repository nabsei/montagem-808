#include "PluginEditor.h"

E808Editor::E808Editor(E808Processor& p)
    : juce::AudioProcessorEditor(&p), processorRef(p),
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel);

    // On-screen keyboard so the synth is playable/testable without a
    // physical MIDI controller connected -- JUCE's standalone wrapper does
    // not provide one itself.
    keyboardComponent.setAvailableRange(24, 72); // C1 to C5, covers 808 range with headroom
    keyboardComponent.setLowestVisibleKey(28);
    addAndMakeVisible(keyboardComponent);

    titleLabel.setText("MONTAGEM 808", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("808 SLIDE  /  ONE-KNOB SYNTH", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, E808LookAndFeel::textDim);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::plain)));
    addAndMakeVisible(subtitleLabel);

    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    amountSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                      juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible(amountSlider);

    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "amount", amountSlider);

    amountValueLabel.setJustificationType(juce::Justification::centred);
    amountValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    amountValueLabel.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    addAndMakeVisible(amountValueLabel);

    footerLabel.setText("AMOUNT", juce::dontSendNotification);
    footerLabel.setJustificationType(juce::Justification::centred);
    footerLabel.setColour(juce::Label::textColourId, E808LookAndFeel::textDim);
    footerLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(footerLabel);

    amountSlider.onValueChange = [this] { updateValueLabel(); repaint(); };
    updateValueLabel();

    noteLabel.setText("--", juce::dontSendNotification);
    noteLabel.setJustificationType(juce::Justification::centred);
    noteLabel.setColour(juce::Label::textColourId, E808LookAndFeel::textDim);
    noteLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    addAndMakeVisible(noteLabel);

    brandLabel.setText("@montagem.808", juce::dontSendNotification);
    brandLabel.setJustificationType(juce::Justification::centredRight);
    brandLabel.setColour(juce::Label::textColourId, E808LookAndFeel::textDim.withAlpha(0.5f));
    brandLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    addAndMakeVisible(brandLabel);

    setResizable(true, true);
    setResizeLimits(400, 360, 900, 720);
    setSize(480, 440);

    startTimerHz(20);
}

E808Editor::~E808Editor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void E808Editor::updateValueLabel()
{
    const int pct = (int)std::round(amountSlider.getValue() * 100.0);
    amountValueLabel.setText(juce::String(pct) + "%", juce::dontSendNotification);
}

juce::String E808Editor::midiNoteToName(int noteNumber)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const int octave = noteNumber / 12 - 1;
    return juce::String(names[noteNumber % 12]) + juce::String(octave);
}

void E808Editor::timerCallback()
{
    const int note = processorRef.lastPlayedNote.load(std::memory_order_relaxed);
    noteLabel.setText(note >= 0 ? midiNoteToName(note) : "--", juce::dontSendNotification);
}

void E808Editor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGradient(E808LookAndFeel::bg.brighter(0.03f), bounds.getCentre(),
                                     E808LookAndFeel::bg.darker(0.15f), bounds.getBottomLeft(), true);
    g.setGradientFill(bgGradient);
    g.fillAll();
}

void E808Editor::resized()
{
    auto full = getLocalBounds();
    keyboardComponent.setBounds(full.removeFromBottom(80));

    auto area = full.reduced(16);

    titleLabel.setBounds(area.removeFromTop(36));
    subtitleLabel.setBounds(area.removeFromTop(20));

    area.removeFromTop(8);
    brandLabel.setBounds(area.removeFromBottom(14));
    footerLabel.setBounds(area.removeFromBottom(18));
    amountValueLabel.setBounds(area.removeFromBottom(28));
    noteLabel.setBounds(area.removeFromBottom(20));

    const int knobSize = juce::jlimit(120, 260, juce::jmin(area.getWidth(), area.getHeight()) - 40);
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
