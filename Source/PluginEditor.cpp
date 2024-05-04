/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),peakFreqSliderAttachment(audioProcessor.apvts, "PD_freq", peakFreqSlider), peakQSliderAttachment(audioProcessor.apvts, "PD_q", peakQSlider), peakGainSliderAttachment(audioProcessor.apvts, "PD_gain", peakGainSlider), lcFreqSliderAttachment(audioProcessor.apvts, "LC_freq", lcFreqSlider), lcSlopeSliderAttachment(audioProcessor.apvts, "LC_slope", lcSlopeSlider), hcFreqSliderAttachment(audioProcessor.apvts, "HC_freq", hcFreqSlider), hcSlopeSliderAttachment(audioProcessor.apvts, "HC_slope", hcSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    
    setSize (1280, 720);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*0.66);
    
    auto lcArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto hcArea = bounds.removeFromRight(bounds.getWidth()*0.5);
    lcFreqSlider.setBounds(lcArea.removeFromTop(lcArea.getHeight()*0.5));
    lcSlopeSlider.setBounds(lcArea);
    hcFreqSlider.setBounds(hcArea.removeFromTop(hcArea.getHeight()*0.5));
    hcSlopeSlider.setBounds(hcArea);
    
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakQSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.5));
    peakGainSlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps() {
    return {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQSlider,
        &lcFreqSlider,
        &lcSlopeSlider,
        &hcFreqSlider,
        &hcSlopeSlider
    };
}
