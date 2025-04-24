#include "PluginEditor.h"

// Width of the whole GUI
static constexpr int WIDTH { 250 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

MainProcessorEditor::MainProcessorEditor(MainProcessor& p) :
    juce::AudioProcessorEditor(p),
    pluginProcessor { p },
    paramEditor(pluginProcessor.getParameterManager(), PARAM_HEIGHT)
{
    addAndMakeVisible(paramEditor);

    // Calculate window height based on number of parameters
    const auto height { pluginProcessor.getParameterManager().getParameters().size() * PARAM_HEIGHT };
    setSize(WIDTH, height);
}

MainProcessorEditor::~MainProcessorEditor()
{
}

void MainProcessorEditor::paint(juce::Graphics&)
{
}

void MainProcessorEditor::resized()
{
    paramEditor.setBounds(getLocalBounds());
}
