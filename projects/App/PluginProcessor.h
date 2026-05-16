#pragma once

#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>

#include "dsp/pitch.h"
#include "dsp/rbuffer.h"

namespace Param
{
    namespace ID
    {
        static const juce::String f { "f" };
    }

    namespace Name
    {
        static const juce::String f { "f" };
    }
}

struct VoiceState {
    int note = 0;
    float speed = 1.0f;
    float position = 0.0f;
};

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

    float f = 0.0f;

    double phaseState = 0.0f;

    static constexpr unsigned MAX_VOICES = 64;
    std::vector<VoiceState> voices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessor)
};
