#pragma once

#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>

#include "dsp/pitch.h"
#include "dsp/rbuffer.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Harmonic2 { "harmonic2" };
        static const juce::String Harmonic3 { "harmonic3" };
        static const juce::String Harmonic4 { "harmonic4" };
        static const juce::String Harmonic5 { "harmonic5" };
    }

    namespace Name
    {
        static const juce::String Enabled { "Enabled" };
        static const juce::String Harmonic2 { "Harmonic2" };
        static const juce::String Harmonic3 { "Harmonic3" };
        static const juce::String Harmonic4 { "Harmonic4" };
        static const juce::String Harmonic5 { "Harmonic5" };
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

    dsp::PitchDetector pitchDetector;
    dsp::rbuffer<float> ibuff;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessor)
};
