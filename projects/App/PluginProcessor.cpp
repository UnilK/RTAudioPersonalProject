#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

#include "math/constants.h"
#include "math/fft.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::Mode,     Param::Name::Mode,      { "Chorus", "FM", "AM" }, 0 },
    { Param::ID::AMGain,     Param::Name::AMGain,     "", 0.1f, 0.0f, 2.f, 0.001f, 0.3f },
    { Param::ID::FMGain,     Param::Name::FMGain,     "", 0.1f, 0.0f, 2.f, 0.001f, 0.3f },
    { Param::ID::Override, Param::Name::Override, "Off", "On", false},
    { Param::ID::Pitch,     Param::Name::Pitch,     "Hz", 10.0f, 1.0f, 1000.0f, 0.1f, 0.3f },

    { Param::ID::Attack,     Param::Name::Attack,     "ms", 0.1f, 0.1f, 500.0f, 0.01f, 0.3f },
    { Param::ID::Decay,     Param::Name::Decay,     "ms", 0.1f, 0.1f, 500.0f, 0.01f, 0.3f },
    { Param::ID::Sustain,     Param::Name::Sustain,     "", 1.0f, 0.0f, 1.0f, 0.01f, 1.0f },
    { Param::ID::Release,     Param::Name::Release,     "ms", 0.1f, 0.1f, 1000.0f, 0.01f, 0.3f },
    { Param::ID::Style,     Param::Name::Style,     { "Linear", "Analog" }, 0 },
};

MainProcessor::MainProcessor() :
    mrta::BaseProcessor(ParameterInfos),
    pitchDetector({ .framerate = 44100.0f }),
    ibuff(1)
{
    math::init_fft(18);

    registerParameterCallback(Param::ID::Mode,
        [this] (float value, bool /*forced*/)
        {
            mode = value;
        });
    registerParameterCallback(Param::ID::AMGain,
        [this] (float value, bool forced)
        {
            amGain.set_target(value, forced);
        });
    registerParameterCallback(Param::ID::FMGain,
        [this] (float value, bool forced)
        {
            fmGain.set_target(value, forced);
        });
    registerParameterCallback(Param::ID::Override,
        [this] (float value, bool /*forced*/)
        {
            doOverridePitch = value == 1.0f;
        });
    registerParameterCallback(Param::ID::Pitch,
        [this] (float value, bool forced)
        {
            overridePitchValue = value;
        });


    
    registerParameterCallback(Param::ID::Attack,
        [this] (float value, bool /*forced*/)
        {
            for(auto &v : voices) v.enveloper.setAttackTime(value);
            attack = value;
        });

    registerParameterCallback(Param::ID::Decay,
        [this] (float value, bool /*forced*/)
        {
            decay = value;
            for(auto &v : voices) v.enveloper.setDecayTime(value);
        });

    registerParameterCallback(Param::ID::Sustain,
        [this] (float value, bool forced)
        {
            sustain.set_target(value, forced);
        });

    registerParameterCallback(Param::ID::Release,
        [this] (float value, bool /*forced*/)
        {
            release = value;
            for(auto &v : voices) v.enveloper.setReleaseTime(value);
        });

    registerParameterCallback(Param::ID::Style,
        [this] (float value, bool /*forced*/)
        {
            analogEnvelopeStyle = value == 1.0f;
            for(auto &v : voices) v.enveloper.setAnalogStyle(value == 1.0f);
        });
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
    
    // just to be safe from 1-off errors in case I make any
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
    const float *x = buffer.getReadPointer(0);

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
        float jumpp = pitchDetector.period * 0.6f;
        if(v.speed > 1.0f && v.position > jumpp){
            while(v.position > 0.0f) v.position -= period;
        } else if(v.speed < 1.0f && v.position < -jumpp){
            while(v.position < 0.0f) v.position += period;
        }

        float d = std::max(1.0f, v.speed);
        float sample = read_sample(&ibuff[0], v.position, d);

        // do fading of pointer jumps
        constexpr float fadeRadius = 16.0f;
        if(v.position < 0.0f && jumpp + v.position < fadeRadius){
            float fade = 0.5f + (jumpp + v.position) / (2.0f * fadeRadius);
            sample = sample * fade + (1.0f - fade) * read_sample(&ibuff[0], v.position + period, d);
        } else if(v.position > 0.0f && jumpp - v.position < fadeRadius){
            float fade = 0.5f + (jumpp - v.position) / (2.0f * fadeRadius);
            sample = sample * fade + (1.0f - fade) * read_sample(&ibuff[0], v.position - period, d);
        }

        return sample;
    };

    int midiEventPos = -1;
    juce::MidiMessage msg;
    juce::MidiBuffer::Iterator midiIt (midi);
    bool nextMidiEvent = midiIt.getNextEvent (msg, midiEventPos);

    float ifs = 1.0f / (float)getSampleRate();

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
                    voices.back().enveloper.start();
                    voices.back().enveloper.setAttackTime(attack);
                    voices.back().enveloper.setDecayTime(decay);
                    voices.back().enveloper.setReleaseTime(release);
                    voices.back().enveloper.setAnalogStyle(analogEnvelopeStyle);
                } else if(it != voices.end()){
                    it->enveloper.start();
                }
            } else if(msg.isNoteOff()){
                int note = msg.getNoteNumber();
                auto it = std::find_if(voices.begin(), voices.end(),
                    [&](VoiceState &v){ return v.note == note; });
                if(it != voices.end()){
                    it->enveloper.end();
                }
            }
            nextMidiEvent = midiIt.getNextEvent(msg, midiEventPos);
        }

        // TODO: something about this stupid? Synths don't seem to overwrite the audio.
        for(int j=0; j<m; j++) buffer.getWritePointer(j)[i] = -buffer.getReadPointer(j)[i];

        float modulationPitch = doOverridePitch ? overridePitchValue : pitchDetector.pitch;

        // clear voices that have turned off
        for(unsigned i=0; i<voices.size(); i++){
            if(voices[i].enveloper.isOff()){
                voices.erase(voices.begin() + i);
                i--;
            }
        }

        {
            float s = sustain.get();
            for(VoiceState &v : voices) v.enveloper.setSustainLevel(s);
        }

        float sample = 0.0f;
        if(mode == 0){
            // The voices are just synth voices
            float tmp;
            for(VoiceState &v : voices) {
                v.enveloper.process(&tmp, 1);
                sample += read_voice(v) * tmp;
            }
            
        } else if(mode == 1){
            // The voices FM-modulate the input
            float speed = 1.0f;
            float g = fmGain.get();
            float tmp;
            for(VoiceState &v : voices){
                v.enveloper.process(&tmp, 1);
                v.position = std::fmod(v.position + v.speed * ifs * modulationPitch, 1.0f);
                speed += std::sin(v.position * 2 * PIF) * g * tmp;
            }
            modulator.speed = speed;
            if(voices.size() == 0){
                modulator.position *= 0.999;
            }
            sample = read_voice(modulator);
        } else {
            // The voices AM-modulate the input
            float amplitude = 1.0f;
            float g = amGain.get();
            float tmp;
            for(VoiceState &v : voices){
                v.enveloper.process(&tmp, 1);
                v.position = std::fmod(v.position + v.speed * ifs * modulationPitch, 1.0f);
                amplitude += std::sin(v.position * 2 * PIF) * g * tmp;
            }
            sample = amplitude * ibuff[0];
        }

        for(int j=0; j<m; j++) buffer.getWritePointer(j)[i] += sample;
    }

}

juce::AudioProcessorEditor* MainProcessor::createEditor()
{
    return new MainProcessorEditor(*this);
}

CREATE_PLUGIN(MainProcessor)
