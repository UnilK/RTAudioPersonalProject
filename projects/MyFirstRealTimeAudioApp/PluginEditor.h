#pragma once

#include "PluginProcessor.h"
#include "GenericParameterEditor.h"

class MainProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    MainProcessorEditor(MainProcessor&);
    ~MainProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MainProcessor& pluginProcessor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessorEditor)
};
