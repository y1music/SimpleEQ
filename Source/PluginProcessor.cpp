/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    updateAllFilters();
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    updateAllFilters();
  
    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;
    
    settings.lcFreq = apvts.getRawParameterValue("LC_freq")->load();
    settings.hcFreq = apvts.getRawParameterValue("HC_freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("PD_freq")->load();
    settings.peakDB_gain = apvts.getRawParameterValue("PD_gain")->load();
    settings.peakQ = apvts.getRawParameterValue("PD_q")->load();
    settings.lcSlope = static_cast<Slope>(apvts.getRawParameterValue("LC_slope")->load());
    settings.hcSlope = static_cast<Slope>(apvts.getRawParameterValue("HC_slope")->load());
    
    return settings;
}

//Updating P/D Filter
void SimpleEQAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings) {
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQ, juce::Decibels::decibelsToGain(chainSettings.peakDB_gain));
    
    //*leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    //*rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void::SimpleEQAudioProcessor::updateCoefficients(Coefficients &old, const Coefficients &replacements) {
    *old = *replacements;
}

//Updating LC
void::SimpleEQAudioProcessor::updateLCFilters(const ChainSettings &chainSettings) {
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lcFreq, getSampleRate(), 2*(chainSettings.lcSlope + 1));
    
    auto& leftLC = leftChain.get<ChainPositions::LowCut>();
    auto& rightLC = rightChain.get<ChainPositions::LowCut>();
    updateCutFilter(leftLC, cutCoefficients, chainSettings.lcSlope);
    updateCutFilter(rightLC, cutCoefficients, chainSettings.lcSlope);
}

//Updating HC
void::SimpleEQAudioProcessor::updateHCFilters(const ChainSettings &chainSettings) {
    auto HCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.hcFreq, getSampleRate(), 2*(chainSettings.hcSlope + 1));
    
    auto& leftHC = leftChain.get<ChainPositions::HighCut>();
    auto& rightHC = rightChain.get<ChainPositions::HighCut>();
    updateCutFilter(leftHC, HCutCoefficients, chainSettings.hcSlope);
    updateCutFilter(rightHC, HCutCoefficients, chainSettings.hcSlope);
}

//Consolidating Updates
void::SimpleEQAudioProcessor::updateAllFilters() {
    auto chainSettings = getChainSettings(apvts);
    
    updatePeakFilter(chainSettings);
    
    updateLCFilters(chainSettings);
    updateHCFilters(chainSettings);
}

//Create Parameters
juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    //Frequency Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LC_freq", 1), "Low Cut Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.0f), 120.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("HC_freq", 1), "High Cut Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.f), 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("PD_freq", 1), "Peak/Dip Frequency", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.0f), 300.0f));
    
    //Parametric Parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("PD_gain", 1), "Peak/Dip Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("PD_q", 1), "Peak/Dip Q", juce::NormalisableRange<float>(0.1f, 24.0f, 0.01f, 1.0f), 1.0f));
    
    //Filter Parameters
    juce::StringArray filterValues;
    for (int i = 0; i<4; ++i) {
        juce::String str;
        str << (48 - 12*i);
        str << " dbB/oct";
        filterValues.add(str);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LC_slope", 1), "Low Cut Slope", filterValues, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("HC_slope", 1), "High Cut Slope", filterValues, 0));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
