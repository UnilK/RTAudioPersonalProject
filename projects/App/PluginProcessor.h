#pragma once


#include <juce_dsp/juce_dsp.h>

#include <BaseProcessor.h>

#include "dsp/pitch.h"
#include "dsp/rbuffer.h"
#include "dsp/EnvelopeGenerator.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Mode { "mode" };
        static const juce::String AMGain { "AMGain" };
        static const juce::String FMGain { "FMGain" };
        static const juce::String Override { "override" };
        static const juce::String Pitch { "pitch" };

        static const juce::String Attack { "attack" };
        static const juce::String Decay { "decay" };
        static const juce::String Sustain { "sustain" };
        static const juce::String Release { "release" };
        static const juce::String Style { "style" };
    }

    namespace Name
    {
        static const juce::String Mode { "Mode" };
        static const juce::String AMGain { "AM gain" };
        static const juce::String FMGain { "FM gain" };
        static const juce::String Override { "Override" };
        static const juce::String Pitch { "A4" };

        static const juce::String Attack { "Attack" };
        static const juce::String Decay { "Decay" };
        static const juce::String Sustain { "Sustain" };
        static const juce::String Release { "Release" };
        static const juce::String Style { "Style" };
    }
}

struct VoiceState {
    int note = 0;
    float speed = 1.0f;
    float position = 0.0f;
    dsp::EnvelopeGenerator enveloper;
};

class SmoothedParameter {
    float value, target, a;
public:
    SmoothedParameter(float initialValue = 0.0f, float decay = 0.001f) : value(initialValue), target(initialValue), a(decay) {}
    void set_target(float targetValue, bool forced = false){
        value = forced ? targetValue : value;
        target = targetValue;
    }
    float get(){
        value = value * (1.0f - a) + a * target;
        return value;
    }
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

    SmoothedParameter amGain, fmGain;
    
    float attack, decay, release;
    SmoothedParameter sustain;
    bool analogEnvelopeStyle;

    bool doOverridePitch;
    float overridePitchValue;

    double phaseState = 0.0f;

    static constexpr unsigned MAX_VOICES = 16;
    std::vector<VoiceState> voices;
    VoiceState modulator;

    int mode = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessor)
};
