#pragma once
#include "PluginProcessor.h"

class AutotuneMqawdProAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit AutotuneMqawdProAudioProcessorEditor(AutotuneMqawdProAudioProcessor&);
    ~AutotuneMqawdProAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }

    AutotuneMqawdProAudioProcessor& processor;
    juce::ComboBox keyBox, scaleBox;
    juce::Label pitchLabel;
    std::vector<std::unique_ptr<juce::Slider>> knobs;
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment, scaleAttachment;

    void addKnob(const juce::String& param, const juce::String& title);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutotuneMqawdProAudioProcessorEditor)
};
