#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

#include "math/constants.h"
#include "math/fft.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::Enabled,   Param::Name::Enabled,   "Off", "On", true },
    { Param::ID::Harmonic2,   Param::Name::Harmonic2,  "", 0.f, 0.f, 2.f, 0.01f, 0.5f },
    { Param::ID::Harmonic3,   Param::Name::Harmonic3,  "", 0.f, 0.f, 2.f, 0.01f, 0.5f },
    { Param::ID::Harmonic4,   Param::Name::Harmonic4,  "", 0.f, 0.f, 2.f, 0.01f, 0.5f },
    { Param::ID::Harmonic5,   Param::Name::Harmonic5,  "", 0.f, 0.f, 2.f, 0.01f, 0.5f },
};

MainProcessor::MainProcessor() :
    mrta::BaseProcessor(ParameterInfos),
    pitchDetector({ .framerate = 44100.0f }),
    ibuff(1)
{
    math::init_fft(18);
    registerParameterCallback(Param::ID::Enabled,
        [this] (float value, bool /*forced*/)
        {
            DBG(Param::Name::Enabled + ": " + juce::String { value });
        });
}

MainProcessor::~MainProcessor()
{
}

void MainProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    juce::uint32 numChannels { static_cast<juce::uint32>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };
    pitchDetector.prepare({ .framerate = (float)sampleRate });

    int radius = pitchDetector.get_reguired_buffer_radius() + 5;
    ibuff.resize(radius * 2, 0.0f);
    ibuff.set_offset(radius);
}

void MainProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    int m = buffer.getNumChannels();
    float *x[64];
    for(int i=0; i<m; i++) x[i] = buffer.getWritePointer(i);

    int n = buffer.getNumSamples();

    static float p = 0.0f, w2 = 0.0;
    float fs = (float)getSampleRate();

    for(int i=0; i<n; i++){
        ibuff.push(x[0][i]);
        pitchDetector.update_period(&ibuff[0]);
        float w = pitchDetector.isVoiced ? pitchDetector.pitch / fs * 2.0f * PIF : 0.0f;
        p = std::fmod(p+w, 2.0f * PIF);
        float s = std::cos(p) * 0.5f;
        for(int j=0; j<m; j++) x[j][i] = s;
    }
}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainProcessorEditor(*this);
}

CREATE_PLUGIN(MainProcessor)
