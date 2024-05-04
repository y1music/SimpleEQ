/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings {
    float lcFreq {0.0f}, hcFreq {0.0f};
    float peakFreq {0.0f}, peakDB_gain {0.0f}, peakQ {1.0f};
    Slope lcSlope {Slope::Slope_12}, hcSlope {Slope::Slope_12};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createParameterLayout()};

private:
    //DSP Namespace Aliases
    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    using FIlter2 = juce::dsp::ProcessorChain<Filter>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions {
        LowCut,
        Peak,
        HighCut
    };
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    template <typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLC, const CoefficientType& cutCoefficients, const ChainSettings& chainSettings) {
        leftLC.template setBypassed<0>(true);
        leftLC.template setBypassed<1>(true);
        leftLC.template setBypassed<2>(true);
        leftLC.template setBypassed<3>(true);
        switch(chainSettings.lcSlope) {
            case Slope_12: {
                *leftLC.template get<0>().coefficients = *cutCoefficients[0];
                leftLC.template setBypassed<0>(false);
            }
            break;
            case Slope_24: {
                *leftLC.template get<0>().coefficients = *cutCoefficients[0];
                leftLC.template setBypassed<0>(false);
                *leftLC.template get<1>().coefficients = *cutCoefficients[1];
                leftLC.template setBypassed<1>(false);
            }
            break;
            case Slope_36: {
                *leftLC.template get<0>().coefficients = *cutCoefficients[0];
                leftLC.template setBypassed<0>(false);
                *leftLC.template get<1>().coefficients = *cutCoefficients[1];
                leftLC.template setBypassed<1>(false);
                *leftLC.template get<2>().coefficients = *cutCoefficients[2];
                leftLC.template setBypassed<2>(false);
            }
            break;
            case Slope_48: {
                *leftLC.template get<0>().coefficients = *cutCoefficients[0];
                leftLC.template setBypassed<0>(false);
                *leftLC.template get<1>().coefficients = *cutCoefficients[1];
                leftLC.template setBypassed<1>(false);
                *leftLC.template get<2>().coefficients = *cutCoefficients[2];
                leftLC.template setBypassed<2>(false);
                *leftLC.template get<3>().coefficients = *cutCoefficients[3];
                leftLC.template setBypassed<3>(false);
            }
            break;
        }
    }
    
    //==============================================================================
    /*
    juce::AudioParameterFloat* LC_freq;
    juce::AudioParameterFloat* HC_freq;
    juce::AudioParameterFloat* PD_freq;
    juce::AudioParameterFloat* PD_gain;
    juce::AudioParameterFloat* PD_q;
    juce::AudioParameterChoice* LC_slope;
    juce::AudioParameterChoice* HC_slope;
     */

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
