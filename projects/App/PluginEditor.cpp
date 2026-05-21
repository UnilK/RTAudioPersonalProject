#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "Style.h"

// Width of the whole GUI
static constexpr int WIDTH { 320 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };
static const int TITLE_HEIGHT { 50 };

static const int MARGIN = 32;

MainProcessorEditor::MainProcessorEditor(MainProcessor& p) :
    juce::AudioProcessorEditor(p),
    pluginProcessor { p },
    leftEditor(pluginProcessor.getParameterManager(), PARAM_HEIGHT, {
        Param::ID::Mode,
        Param::ID::AMGain,
        Param::ID::FMGain,
        Param::ID::Override,
        Param::ID::Pitch,
    }),
    rightEditor(pluginProcessor.getParameterManager(), PARAM_HEIGHT, {
        Param::ID::Attack,
        Param::ID::Decay,
        Param::ID::Sustain,
        Param::ID::Release,
        Param::ID::Style,
    })
{
    style = std::make_unique<Style>();
    juce::LookAndFeel::setDefaultLookAndFeel(style.get());
    setLookAndFeel(style.get());

    addAndMakeVisible(title);
    addAndMakeVisible(leftEditor);
    addAndMakeVisible(rightEditor);

    title.setText("VOICE SYNTHEX", juce::NotificationType::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);

    // Calculate window height based on number of parameters
    setSize(2 * WIDTH + MARGIN, 5 * PARAM_HEIGHT + TITLE_HEIGHT);
}

MainProcessorEditor::~MainProcessorEditor()
{
}

void MainProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(palette.bg);
    g.setColour(palette.lwhite);
    auto b = getLocalBounds();
    g.fillRect(b.getWidth()/2, TITLE_HEIGHT, 2, b.getHeight());
}

void MainProcessorEditor::resized()
{
    juce::FlexBox fb, fbd;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.items.add(juce::FlexItem(title).withFlex(0, 0, TITLE_HEIGHT));
    fb.items.add(juce::FlexItem(fbd).withFlex(1));
    fbd.items.add(juce::FlexItem(leftEditor).withFlex(1).withMargin({0, MARGIN, 0, 0}));
    fbd.items.add(juce::FlexItem(rightEditor).withFlex(1));
    fb.performLayout(getLocalBounds());
}
