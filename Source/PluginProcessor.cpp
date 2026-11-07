#include "PluginProcessor.h"
#include "PluginEditor.h"

AutotuneMqawdProAudioProcessor::AutotuneMqawdProAudioProcessor()
: AudioProcessor(BusesProperties()
      .withInput("Input", juce::AudioChannelSet::stereo(), true)
      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
AutotuneMqawdProAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APB = juce::AudioParameterBool;
    using APC = juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<APB>("tuneOn", "Pitch Correction", true));
    p.push_back(std::make_unique<APC>("key", "Key", juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 0));
    p.push_back(std::make_unique<APC>("scale", "Scale", juce::StringArray{"Major","Minor","Chromatic"}, 0));
    p.push_back(std::make_unique<APF>("retune", "Retune Speed", juce::NormalisableRange<float>(1.0f, 200.0f, 0.1f), 35.0f));
    p.push_back(std::make_unique<APF>("humanize", "Humanize", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 25.0f));

    p.push_back(std::make_unique<APF>("low", "Low EQ", juce::NormalisableRange<float>(-12.0f,12.0f,0.1f), 0.0f));
    p.push_back(std::make_unique<APF>("mid", "Mid EQ", juce::NormalisableRange<float>(-12.0f,12.0f,0.1f), 0.0f));
    p.push_back(std::make_unique<APF>("high", "High EQ", juce::NormalisableRange<float>(-12.0f,12.0f,0.1f), 0.0f));
    p.push_back(std::make_unique<APF>("air", "Air EQ", juce::NormalisableRange<float>(-12.0f,12.0f,0.1f), 0.0f));

    p.push_back(std::make_unique<APF>("chorusMix", "Chorus", juce::NormalisableRange<float>(0.0f,1.0f,0.001f), 0.0f));
    p.push_back(std::make_unique<APF>("reverbMix", "Reverb", juce::NormalisableRange<float>(0.0f,1.0f,0.001f), 0.15f));
    p.push_back(std::make_unique<APF>("saturation", "Saturation", juce::NormalisableRange<float>(0.0f,1.0f,0.001f), 0.0f));
    p.push_back(std::make_unique<APF>("output", "Output Gain", juce::NormalisableRange<float>(-24.0f,12.0f,0.1f), 0.0f));

    return { p.begin(), p.end() };
}

bool AutotuneMqawdProAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || l.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void AutotuneMqawdProAudioProcessor::prepareToPlay(double sr, int block)
{
    currentSampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, (juce::uint32) getTotalNumOutputChannels() };
    lowFilter.prepare(spec); midFilter.prepare(spec); highFilter.prepare(spec); airFilter.prepare(spec);
    chorusL.prepare({sr,(juce::uint32)block,1}); chorusR.prepare({sr,(juce::uint32)block,1});
    chorusL.reset(); chorusR.reset();
    chorusL.setRate(0.35f); chorusR.setRate(0.42f);
    chorusL.setDepth(0.25f); chorusR.setDepth(0.28f);
    chorusL.setCentreDelay(8.0f); chorusR.setCentreDelay(11.0f);
    reverb.reset();
    reverb.setSampleRate(sr);
    outputGain.prepare(spec);
    updateFilters();
}

void AutotuneMqawdProAudioProcessor::updateFilters()
{
    auto low = apvts.getRawParameterValue("low")->load();
    auto mid = apvts.getRawParameterValue("mid")->load();
    auto high = apvts.getRawParameterValue("high")->load();
    auto air = apvts.getRawParameterValue("air")->load();

    *lowFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, 120.0, 0.707, juce::Decibels::decibelsToGain(low));
    *midFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 1200.0, 0.8, juce::Decibels::decibelsToGain(mid));
    *highFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 6000.0, 0.707, juce::Decibels::decibelsToGain(high));
    *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 12000.0, 0.707, juce::Decibels::decibelsToGain(air));
}

float AutotuneMqawdProAudioProcessor::estimatePitch(const float* x, int n) const
{
    if (n < 256) return 0.0f;
    int minLag = juce::jmax(2, (int)(currentSampleRate / 1000.0));
    int maxLag = juce::jmin(n / 2, (int)(currentSampleRate / 70.0));
    float best = 0.0f; int bestLag = 0;

    for (int lag = minLag; lag < maxLag; ++lag)
    {
        float sum = 0.0f, energy = 1.0e-9f;
        for (int i = 0; i + lag < n; i += 2)
        {
            sum += x[i] * x[i + lag];
            energy += x[i] * x[i];
        }
        float score = sum / energy;
        if (score > best) { best = score; bestLag = lag; }
    }
    return (best > 0.25f && bestLag > 0) ? (float)(currentSampleRate / bestLag) : 0.0f;
}

void AutotuneMqawdProAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateFilters();

    if (buffer.getNumChannels() > 0)
        detectedFrequency.store(estimatePitch(buffer.getReadPointer(0), buffer.getNumSamples()));

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    lowFilter.process(context); midFilter.process(context); highFilter.process(context); airFilter.process(context);

    float sat = apvts.getRawParameterValue("saturation")->load();
    if (sat > 0.001f)
        for (int ch=0; ch<buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i=0;i<buffer.getNumSamples();++i)
                d[i] = std::tanh(d[i] * (1.0f + sat * 8.0f));
        }

    float cmix = apvts.getRawParameterValue("chorusMix")->load();
    if (cmix > 0.001f)
    {
        for (int ch=0; ch<buffer.getNumChannels(); ++ch)
        {
            juce::AudioBuffer<float> temp(1, buffer.getNumSamples());
            temp.copyFrom(0,0,buffer,ch,0,buffer.getNumSamples());
            juce::dsp::AudioBlock<float> b(temp);
            juce::dsp::ProcessContextReplacing<float> c(b);
            (ch == 0 ? chorusL : chorusR).process(c);
            buffer.addFrom(ch,0,temp,0,0,buffer.getNumSamples(),cmix);
        }
    }

    juce::Reverb::Parameters rp;
    rp.roomSize = 0.45f; rp.damping = 0.5f; rp.width = 1.0f;
    rp.wetLevel = apvts.getRawParameterValue("reverbMix")->load();
    rp.dryLevel = 1.0f;
    reverb.setParameters(rp);
    reverb.processStereo(buffer.getWritePointer(0),
                         buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : buffer.getWritePointer(0),
                         buffer.getNumSamples());

    outputGain.setGainDecibels(apvts.getRawParameterValue("output")->load());
    outputGain.process(context);
}

juce::AudioProcessorEditor* AutotuneMqawdProAudioProcessor::createEditor()
{
    return new AutotuneMqawdProAudioProcessorEditor(*this);
}

void AutotuneMqawdProAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void AutotuneMqawdProAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
