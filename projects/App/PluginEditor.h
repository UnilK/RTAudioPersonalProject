#pragma once

#include "PluginProcessor.h"
#include "GenericParameterEditor.h"

class Style;

class MainProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    MainProcessorEditor(MainProcessor&);
    ~MainProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    std::unique_ptr<Style> style;
    MainProcessor& pluginProcessor;

    juce::Label title;
    mrta::GenericParameterEditor leftEditor, rightEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainProcessorEditor)
};
