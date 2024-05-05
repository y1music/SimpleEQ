/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void L_n_F::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) {
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(Colour::fromRGB(224, 221, 213));
    g.fillEllipse(bounds);
    g.setColour(Colour::fromRGB(255, 254, 252));
    g.drawEllipse(bounds, 2.f);
    
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&s)) {
        auto centre = bounds.getCentre();
        
        Path p;
        Rectangle<float> r;
        r.setLeft(centre.getX()-2);
        r.setRight(centre.getX()+2);
        r.setTop(bounds.getY());
        r.setBottom(centre.getY() - rswl->getTextHeight() * 1.5);
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, centre.getX(), centre.getY()));
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, rswl->getTextHeight() +2);
        r.setCentre(bounds.getCentre());
        g.setColour(Colours::black);
        g.fillRect(r);
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), Justification::centred, 1);
        
    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g) {
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    float sliderPosPropVar = jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0);
    
    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());
    //g.setColour(Colours::yellow);
    //g.drawRect(getSliderBounds());
    
    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), sliderPosPropVar, startAng, endAng, *this);
    
    auto centre = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth();
    
    g.setColour(Colour::fromRGB(255, 255, 255));
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i=0; i<numChoices; i++) {
        auto pos =  labels[i].position;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto angle = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = centre.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, angle);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() - getTextHeight());
        g.drawFittedText(str, r.toNearestInt(), Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    using namespace juce;
    
    auto bounds = getLocalBounds();
    
    auto size = jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 2;
    
    Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const{
    using namespace juce;
    if (auto* choiceParam = dynamic_cast<AudioParameterChoice*>(param)) {
        return choiceParam->getCurrentChoiceName();
    }
    
    String str;
    
    if (auto* floatParam = dynamic_cast<AudioParameterFloat*>(param)) {
        float val = getValue();
        
        str = String(val, 0);
        
    } else {
        jassertfalse; //This should not happen
    }
    
    if (suffix.isNotEmpty()) {
        str << " ";
        str << suffix;
    } else {
        str << " ";
        str << "Q";
    }
    
    return str;
}

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), peakFreqSlider(*audioProcessor.apvts.getParameter("PD_freq"), "Hz"), peakQSlider(*audioProcessor.apvts.getParameter("PD_q"), ""), peakGainSlider(*audioProcessor.apvts.getParameter("PD_gain"), "dB"), lcFreqSlider(*audioProcessor.apvts.getParameter("LC_freq"), "Hz"), lcSlopeSlider(*audioProcessor.apvts.getParameter("LC_slope"), "dB/Oct"), hcFreqSlider(*audioProcessor.apvts.getParameter("HC_freq"), "Hz"), hcSlopeSlider(*audioProcessor.apvts.getParameter("HC_slope"), "dB/Oct"), peakFreqSliderAttachment(audioProcessor.apvts, "PD_freq", peakFreqSlider), peakQSliderAttachment(audioProcessor.apvts, "PD_q", peakQSlider), peakGainSliderAttachment(audioProcessor.apvts, "PD_gain", peakGainSlider), lcFreqSliderAttachment(audioProcessor.apvts, "LC_freq", lcFreqSlider), lcSlopeSliderAttachment(audioProcessor.apvts, "LC_slope", lcSlopeSlider), hcFreqSliderAttachment(audioProcessor.apvts, "HC_freq", hcFreqSlider), hcSlopeSliderAttachment(audioProcessor.apvts, "HC_slope", hcSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    peakQSlider.labels.add({0.f, "0.1"});
    peakQSlider.labels.add({1.f, "24"});
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});
    lcFreqSlider.labels.add({0.f, "20Hz"});
    lcFreqSlider.labels.add({1.f, "20kHz"});
    lcSlopeSlider.labels.add({0.f, "48dB/Oct"});
    lcSlopeSlider.labels.add({1.f, "12dB/Oct"});
    hcFreqSlider.labels.add({0.f, "20Hz"});
    hcFreqSlider.labels.add({1.f, "20kHz"});
    hcSlopeSlider.labels.add({0.f, "48dB/Oct"});
    hcSlopeSlider.labels.add({1.f, "12dB/Oct"});
    
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
    using namespace juce;
    
    g.fillAll (Colour::fromRGB(38, 38, 38));
    
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
