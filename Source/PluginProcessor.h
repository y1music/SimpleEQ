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
    
    bool lcBypassed = false;
    bool pdBypassed = false;
    bool hcBypassed = false;
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//DSP Namespace Aliases
using Filter = juce::dsp::IIR::Filter<float>;
using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

enum ChainPositions {
    LowCut,
    Peak,
    HighCut
};

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
    
    MonoChain leftChain, rightChain;
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    template<int Index, typename ChainType, typename CoefficientType>
    void updateSwitchCase(ChainType& chain, const CoefficientType& coefficients) {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }
    
    template <typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLC, const CoefficientType& cutCoefficients, const Slope& lcSlope) {
        leftLC.template setBypassed<0>(true);
        leftLC.template setBypassed<1>(true);
        leftLC.template setBypassed<2>(true);
        leftLC.template setBypassed<3>(true);
        
        switch(lcSlope) {
            case Slope_48: {
                updateSwitchCase<3>(leftLC, cutCoefficients);
            }
            break;
            case Slope_36: {
                updateSwitchCase<2>(leftLC, cutCoefficients);
            }
            break;
            case Slope_24: {
                updateSwitchCase<1>(leftLC, cutCoefficients);
            }
            break;
            case Slope_12: {
                updateSwitchCase<0>(leftLC, cutCoefficients);
            }
            break;
        }
    }
    
    void updateLCFilters (const ChainSettings& chainSettings);
    void updateHCFilters (const ChainSettings& chainSettings);
    void updateAllFilters ();
    
    //==============================================================================
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
