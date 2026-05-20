/*
 * Basic styling
 * Copyright (C) 2024 Unto Karila
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
*/

#include <BinaryData.h>

#include "Style.h"

Palette palette {
    .bg = juce::Colour(0xFF201918),
    .lbg = juce::Colour(0xFF585454),
    .red = juce::Colour(0xFFCC321F),
    .lred = juce::Colour(0xFFE14444),
    .green = juce::Colour(0xFF429B18),
    .lgreen = juce::Colour(0xFF7DD625),
    .yellow = juce::Colour(0xFFC2B11A),
    .lyellow = juce::Colour(0xFFF7E13A),
    .blue = juce::Colour(0xFF3B7ABD),
    .lblue = juce::Colour(0xFF6895C5),
    .violet = juce::Colour(0xFFAF64AF),
    .lviolet = juce::Colour(0xFFB686B0),
    .cyan = juce::Colour(0xFF24B2A4),
    .lcyan = juce::Colour(0xFF46C2D2),
    .white = juce::Colour(0xFFB6B3A9),
    .lwhite = juce::Colour(0xFFEEE8E0),
    .gray = juce::Colour(0xFF32302F),
    .lgray = juce::Colour(0xFF585454)
};

float fontSize = 18.0f;

Style::Style(){
    
    juce::LookAndFeel_V4::ColourScheme scheme {
        palette.bg,     // windowBackground
        palette.gray,     // widgetBackground
        palette.bg,    // menuBackground
        palette.lwhite, // outline
        palette.lwhite, // defaultText
        palette.bg,     // defaultFill
        palette.lwhite, // highlightedText
        palette.lbg,    // highlightedFill
        palette.lwhite  // menuText
    };

    setColourScheme(scheme);

    setColour(juce::Slider::thumbColourId, palette.lblue);
    setColour(juce::Slider::trackColourId, palette.blue);
    setColour(juce::Slider::backgroundColourId, palette.lbg);
    setColour(juce::CaretComponent::caretColourId, palette.lwhite);
    setColour(juce::TooltipWindow::backgroundColourId, palette.gray);
}

juce::Typeface::Ptr Style::getTypefaceForFont(const juce::Font &font){
    return juce::Typeface::createSystemTypefaceFor(
        BinaryData::UbuntuMonoR_ttf, BinaryData::UbuntuMonoR_ttfSize);
}

juce::Font Style::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return { juce::FontOptions{ juce::jmin (fontSize, (float) buttonHeight * 0.8f) } };
}

void Style::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool){

    using namespace juce;

    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

void Style::drawButtonBackground(
    juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{

    using namespace juce;

    auto cornerSize = 1.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColour (button.findColour (ComboBox::outlineColourId));
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (button.findColour (ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

juce::Font Style::getLabelFont (juce::Label& label)
{
    return {juce::FontOptions{fontSize}};
}

void Style::drawCornerResizer(juce::Graphics& g, int w, int h, bool, bool)
{
    auto lineThickness = 1.0f;

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.setColour (palette.lwhite);

        g.drawLine ((float) w * i,
                    (float) h + 1.0f,
                    (float) w + 1.0f,
                    (float) h * i,
                    lineThickness);

        g.setColour (palette.lwhite);

        g.drawLine ((float) w * i + lineThickness,
                    (float) h + 1.0f,
                    (float) w + 1.0f,
                    (float) h * i + lineThickness,
                    lineThickness);
    }
}


///////////////////////////////////////////////////////////////////////////////


int SliderStyle::getSliderThumbRadius (juce::Slider& slider)
{
    return juce::jmin ((int)fontSize, slider.isHorizontal() ? static_cast<int> ((float) slider.getHeight() * 0.8f)
                                           : static_cast<int> ((float) slider.getWidth()  * 0.8));
}

void SliderStyle::drawLabel (juce::Graphics& g, juce::Label& label)
{
    using namespace juce;

    g.fillAll (label.findColour (Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }

    float cornerSize = 1.0f;
    auto bounds = label.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    Path path;
    path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                bounds.getWidth(), bounds.getHeight(),
                                cornerSize, cornerSize,
                                true, true, true, true);

    g.setColour (palette.lwhite);
    g.strokePath (path, PathStrokeType (1.0f));
}

class SliderLabelComp2 final : public juce::Label
{
public:

    SliderLabelComp2() : juce::Label ({}, {}) {}

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) {}

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }
};

juce::Label* SliderStyle::createSliderTextBox(juce::Slider& slider)
{
    using namespace juce;

    auto l = new SliderLabelComp2();

    l->setJustificationType (Justification::centred);
    l->setKeyboardType (TextInputTarget::decimalKeyboard);

    l->setColour (Label::textColourId, palette.lwhite);
    l->setColour (Label::backgroundColourId,
                  (slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                            ? Colours::transparentBlack
                            : palette.bg);
    l->setColour (Label::outlineColourId, palette.lwhite);
    l->setColour (TextEditor::textColourId, palette.lwhite);
    l->setColour (TextEditor::backgroundColourId,
                  palette.bg
                        .withAlpha ((slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                                        ? 0.7f : 1.0f));
    l->setColour (TextEditor::outlineColourId, palette.lwhite);
    l->setColour (TextEditor::highlightColourId, palette.lbg);

    return l;
}


///////////////////////////////////////////////////////////////////////////////


void SliderStyleMiddle::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    using namespace juce;

    auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

    Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

    Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                            slider.isHorizontal() ? startPoint.y : (float) y);

    Path backgroundTrack;
    backgroundTrack.startNewSubPath (startPoint);
    backgroundTrack.lineTo (endPoint);
    g.setColour (slider.findColour (Slider::backgroundColourId));
    g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

    Path valueTrack;
    Point<float> maxPoint, thumbPoint;

    auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
    auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;

    Point<float> minPoint ((float) x + (float) width * 0.5f, (float) y + (float) height * 0.5f);
    maxPoint = { kx, ky };

    auto thumbWidth = getSliderThumbRadius (slider);

    valueTrack.startNewSubPath (minPoint);
    valueTrack.lineTo (maxPoint);
    g.setColour (slider.findColour (Slider::trackColourId));
    g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

    g.setColour (slider.findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (maxPoint));
}


///////////////////////////////////////////////////////////////////////////////



void SelectorButtonStyle::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool){

    using namespace juce;

    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 5, 1);
}


///////////////////////////////////////////////////////////////////////////////



void TextEntryStyle::drawLabel (juce::Graphics& g, juce::Label& label)
{
    using namespace juce;

    g.fillAll (label.findColour (Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }

    float cornerSize = 1.0f;
    auto bounds = label.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    Path path;
    path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                bounds.getWidth(), bounds.getHeight(),
                                cornerSize, cornerSize,
                                true, true, true, true);

    g.setColour (palette.lwhite);
    g.strokePath (path, PathStrokeType (1.0f));
}
