#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

#include "math/constants.h"
#include "math/fft.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::f,   Param::Name::f,  "", 1.f, 0.5f, 2.f, 0.01f, 1.0f },
};

MainProcessor::MainProcessor() :
    mrta::BaseProcessor(ParameterInfos),
    pitchDetector({ .framerate = 44100.0f }),
    ibuff(1)
{
    math::init_fft(18);

    registerParameterCallback(Param::ID::f,
        [this] (float value, bool forced){ f = value; });
}

MainProcessor::~MainProcessor()
{
}

static constexpr int N_RESAMPLE = 12;

void MainProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    juce::uint32 numChannels { static_cast<juce::uint32>(
        std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };
    pitchDetector.prepare({ .framerate = (float)sampleRate });

    int radius = pitchDetector.get_reguired_buffer_radius();
    radius = std::max(radius, pitchDetector.get_reguired_buffer_radius() / 2 + 1 + N_RESAMPLE);
    
    // just to be safe
    radius += 1;
    
    ibuff.resize(radius * 2, 0.0f);
    ibuff.set_offset(radius);

    voices.reserve(MAX_VOICES);
}

void MainProcessor::process(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    int n = buffer.getNumSamples();
    int m = std::min(buffer.getNumChannels(), 2);
    float *x = buffer.getWritePointer(0);

    auto sinc = [&](float x){ return x*x < 1e-12f ? 1.0f : sin(x * PIF) / (x * PIF); };
    auto window = [&](float x){ return (0.5f + 0.5f * cos(x)); };

    // Read samples using sinc interpolation (basic resampling)
    auto read_sample = [&](const float *x, float position, float width){
        float sum = 0.0f;
        int l = (int)std::ceil(position - N_RESAMPLE);
        int r = (int)std::floor(position + N_RESAMPLE);
        float dsinc = 1.0f / width;
        constexpr float dwin = PIF / N_RESAMPLE;
        for(int i=l; i<=r; i++){
            float p = position - i;
            sum += x[i] * sinc(p * dsinc) * window(p * dwin);
        }
        return sum * dsinc;
    };

    auto read_voice = [&](VoiceState &v){
        
        v.position += v.speed - 1.0f;

        // The read pointer loops around the center point of the buffer
        float period = pitchDetector.period;
        float halfp = pitchDetector.period / 2.0f;
        if(v.speed > 1.0f && v.position > halfp){
            while(v.position > 0.0f) v.position -= period;
        } else if(v.speed < 1.0f && v.position < -halfp){
            while(v.position < 0.0f) v.position += period;
        }

        float d = std::max(1.0f, v.speed);
        float sample = read_sample(&ibuff[0], v.position, d);

        // do fading of pointer jumps
        constexpr float fadeRadius = 16.0f;
        if(v.position < 0.0f && halfp + v.position < fadeRadius){
            float fade = 0.5f + (halfp + v.position) / (2.0f * fadeRadius);
            sample = sample * fade + (1.0f - fade) * read_sample(&ibuff[0], v.position + period, d);
        } else if(v.position > 0.0f && halfp - v.position < fadeRadius){
            float fade = 0.5f + (halfp - v.position) / (2.0f * fadeRadius);
            sample = sample * fade + (1.0f - fade) * read_sample(&ibuff[0], v.position - period, d);
        }

        return sample;
    };

    int midiEventPos = -1;
    juce::MidiMessage msg;
    juce::MidiBuffer::Iterator midiIt (midi);
    bool nextMidiEvent = midiIt.getNextEvent (msg, midiEventPos);

    for(int i=0; i<n; i++){
        ibuff.push(x[i]);
        pitchDetector.update_period(&ibuff[0]);

        while(nextMidiEvent && midiEventPos == i){
            if(msg.isNoteOn()){
                int note = msg.getNoteNumber();
                auto it = std::find_if(voices.begin(), voices.end(),
                    [&](VoiceState &v){ return v.note == note; });
                if(it == voices.end() && voices.size() < MAX_VOICES){
                    float relative_freq = msg.getMidiNoteInHertz(note, 1.0);
                    voices.push_back({ .note = note, .speed = relative_freq, .position = 0.0f });
                }
            } else if(msg.isNoteOff()){
                int note = msg.getNoteNumber();
                auto it = std::find_if(voices.begin(), voices.end(),
                    [&](VoiceState &v){ return v.note == note; });
                if(it != voices.end()){
                    voices.erase(it);
                }
            }
            nextMidiEvent = midiIt.getNextEvent(msg, midiEventPos);
        }

        // TODO: something about this stupid
        x[i] = -x[i];
        for(VoiceState &v : voices) x[i] += read_voice(v);

        for(int j=0; j<m; j++) buffer.getWritePointer(j)[i] = x[i];
    }

}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainProcessorEditor(*this);
}

CREATE_PLUGIN(MainProcessor)
