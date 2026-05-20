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

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct Palette {
    juce::Colour bg, lbg;
    juce::Colour red, lred;
    juce::Colour green, lgreen;
    juce::Colour yellow, lyellow;
    juce::Colour blue, lblue;
    juce::Colour violet, lviolet;
    juce::Colour cyan, lcyan;
    juce::Colour white, lwhite;
    juce::Colour gray, lgray;
};

extern Palette palette;
extern float fontSize;

class Style : public juce::LookAndFeel_V4 {

public:

    Style();

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font &font) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override;

    void drawButtonBackground(
        juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont(juce::Label& label) override;

    void drawCornerResizer(juce::Graphics& g, int w, int h, bool, bool) override;
};



class SliderStyle : public Style {

public:

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    int getSliderThumbRadius (juce::Slider& slider) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

};



class SliderStyleMiddle : public SliderStyle {

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};



class SelectorButtonStyle : public Style {

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override;

};



class TextEntryStyle : public Style {

public:

    void drawLabel (juce::Graphics& g, juce::Label& label) override;

};
