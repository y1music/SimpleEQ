/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct L_n_F : juce::LookAndFeel_V4 {
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    
};

struct RotarySliderWithLabels : juce::Slider {
    //Constructor + Destructor
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox), param(&rap), suffix(unitSuffix) {
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos {
        float position;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    //public inits
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const {return 14;}
    juce::String getDisplayString() const;
    
    //private variables
    private:
    L_n_F lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
    
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;
    
    RotarySliderWithLabels peakFreqSlider,
    peakQSlider,
    peakGainSlider,
    lcFreqSlider,
    lcSlopeSlider,
    hcFreqSlider,
    hcSlopeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment peakFreqSliderAttachment,
    peakQSliderAttachment,
    peakGainSliderAttachment,
    lcFreqSliderAttachment,
    lcSlopeSliderAttachment,
    hcFreqSliderAttachment,
    hcSlopeSliderAttachment;
    
    juce::ToggleButton lcBypassButton, pdBypassButton, hcBypassButton;
    
    std::vector<juce::Component*> getComps();
    
    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
