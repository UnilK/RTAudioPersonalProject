#pragma once

#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Drive { "drive" };
        static const juce::String Frequency { "frequency" };
        static const juce::String Resonance { "resonance" };
        static const juce::String Mode { "mode" };
        static const juce::String PostGain { "post_gain" };
    }

    namespace Name
    {
        static const juce::String Enabled { "Enabled" };
        static const juce::String Drive { "Drive" };
        static const juce::String Frequency { "Frequency" };
        static const juce::String Resonance { "Resonance" };
        static const juce::String Mode { "Mode" };
        static const juce::String PostGain { "Post-Gain" };
    }
}

class MainProcessor final : public mrta::BaseProcessor
{
public:
    MainProcessor();
    ~MainProcessor() override;

    // Called before processing starts
    void prepare(double sampleRate, int samplesPerBlock) override;

    // Audio stream callback
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Creates the GUI
    juce::AudioProcessorEditor* createEditor() override;

private:
    juce::dsp::LadderFilter<float> filter;
    juce::SmoothedValue<float> outputGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessor)
};
