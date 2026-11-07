#include "PluginEditor.h"

AutotuneMqawdProAudioProcessorEditor::AutotuneMqawdProAudioProcessorEditor(AutotuneMqawdProAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p)
{
    setSize(1100, 700);

    keyBox.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1);
    scaleBox.addItemList({"Major","Minor","Chromatic"}, 1);
    addAndMakeVisible(keyBox); addAndMakeVisible(scaleBox);
    keyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts,"key",keyBox);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts,"scale",scaleBox);

    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchLabel.setFont(juce::FontOptions(48.0f, juce::Font::bold));
    pitchLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(pitchLabel);

    for (auto [id, title] : std::vector<std::pair<juce::String,juce::String>>{
        {"retune","RETUNE"},{"humanize","HUMANIZE"},
        {"low","LOW"},{"mid","MID"},{"high","HIGH"},{"air","AIR"},
        {"chorusMix","CHORUS"},{"reverbMix","REVERB"},{"saturation","SATURATION"},{"output","OUTPUT"}})
        addKnob(id,title);

    startTimerHz(20);
}

void AutotuneMqawdProAudioProcessorEditor::addKnob(const juce::String& param, const juce::String& title)
{
    auto s = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    s->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8b2cff));
    s->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff18233a));
    addAndMakeVisible(*s);
    attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,param,*s));
    knobs.push_back(std::move(s));

    auto l = std::make_unique<juce::Label>();
    l->setText(title, juce::dontSendNotification);
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, juce::Colour(0xffd8ddff));
    addAndMakeVisible(*l);
    labels.push_back(std::move(l));
}

void AutotuneMqawdProAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff090d15));
    g.setColour(juce::Colour(0xff121a29));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10), 18.0f);

    g.setColour(juce::Colour(0xff9a3cff));
    g.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    g.drawText("AUTOTUNE MQAWD PRO", 35, 25, 600, 45, juce::Justification::left);

    g.setColour(juce::Colour(0xff9aa8c7));
    g.setFont(juce::FontOptions(16.0f));
    g.drawText("PROFESSIONAL VOCAL PROCESSING • VST3 PROTOTYPE", 38, 70, 600, 25, juce::Justification::left);

    auto hz = processor.getDetectedFrequency();
    juce::String text = hz > 0.0f ? juce::String(hz,1) + " Hz" : "-- Hz";
    pitchLabel.setText(text, juce::dontSendNotification);

    g.setColour(juce::Colour(0xff0d1320));
    g.fillRoundedRectangle(370, 120, 360, 190, 20);
    g.setColour(juce::Colour(0xff00d8ff));
    g.drawRoundedRectangle(370,120,360,190,20,2);
}

void AutotuneMqawdProAudioProcessorEditor::resized()
{
    keyBox.setBounds(35,130,120,35);
    scaleBox.setBounds(170,130,150,35);
    pitchLabel.setBounds(390,160,320,100);

    int startX=30, y=350, w=100, gap=15;
    for (size_t i=0;i<knobs.size();++i)
    {
        int row = (int)i / 6, col = (int)i % 6;
        int x = startX + col*(w+gap);
        int yy = y + row*170;
        knobs[i]->setBounds(x,yy,w,115);
        labels[i]->setBounds(x,yy+115,w,30);
    }
}
