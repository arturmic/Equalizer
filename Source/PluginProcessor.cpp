/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEqAudioProcessor::SimpleEqAudioProcessor()
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

SimpleEqAudioProcessor::~SimpleEqAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEqAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	//Initialising the ProcessorChain
	juce::dsp::ProcessSpec spec;
	spec.maximumBlockSize = samplesPerBlock;
	spec.sampleRate = sampleRate;
	spec.numChannels = 1;

	leftChain.prepare(spec);
	rightChain.prepare(spec);

	// Create Coefficients
	auto chainSettings = getChainSettings(apvts);

	updatePeakFilter(chainSettings);

	auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
		sampleRate,
		2 * (chainSettings.lowCutSlope + 1));

	auto& leftLowCut = leftChain.get<chainPositions::LowCut>();

	leftLowCut.setBypassed<0>(true);
	leftLowCut.setBypassed<1>(true);
	leftLowCut.setBypassed<2>(true);
	leftLowCut.setBypassed<3>(true);

	switch (chainSettings.lowCutSlope)
	{
	case Slope_12:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		break;
	}
	case Slope_24:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		break;
	}
	case Slope_36:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<2>().coefficients = *cutCoefficients[2];
		leftLowCut.setBypassed<2>(false);
		break;
	}
	case Slope_48:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[2];
		leftLowCut.setBypassed<2>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[3];
		leftLowCut.setBypassed<3>(false);
		break;
	}

	}

	auto& rightLowCut = rightChain.get<chainPositions::LowCut>();

	rightLowCut.setBypassed<0>(true);
	rightLowCut.setBypassed<1>(true);
	rightLowCut.setBypassed<2>(true);
	rightLowCut.setBypassed<3>(true);

	switch (chainSettings.lowCutSlope)
	{
	case Slope_12:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		break;
	}
	case Slope_24:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		break;
	}
	case Slope_36:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<2>().coefficients = *cutCoefficients[2];
		rightLowCut.setBypassed<2>(false);
		break;
	}
	case Slope_48:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[2];
		rightLowCut.setBypassed<2>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[3];
		rightLowCut.setBypassed<3>(false);
		break;
	}
	}

}

void SimpleEqAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
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

void SimpleEqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// Running Audio through the ProcessorChain

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	auto chainSettings = getChainSettings(apvts);

	updatePeakFilter(chainSettings);


	auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
		getSampleRate(),
		2 * (chainSettings.lowCutSlope + 1));

	auto& leftLowCut = leftChain.get<chainPositions::LowCut>();

	leftLowCut.setBypassed<0>(true);
	leftLowCut.setBypassed<1>(true);
	leftLowCut.setBypassed<2>(true);
	leftLowCut.setBypassed<3>(true);

	switch (chainSettings.lowCutSlope)
	{
	case Slope_12:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		break;
	}
	case Slope_24:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		break;
	}
	case Slope_36:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<1>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<2>().coefficients = *cutCoefficients[2];
		leftLowCut.setBypassed<2>(false);
		break;
	}
	case Slope_48:
	{
		*leftLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[1];
		leftLowCut.setBypassed<1>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[2];
		leftLowCut.setBypassed<2>(false);
		*leftLowCut.get<0>().coefficients = *cutCoefficients[3];
		leftLowCut.setBypassed<3>(false);
		break;
	}

	}

	auto& rightLowCut = rightChain.get<chainPositions::LowCut>();

	rightLowCut.setBypassed<0>(true);
	rightLowCut.setBypassed<1>(true);
	rightLowCut.setBypassed<2>(true);
	rightLowCut.setBypassed<3>(true);

	switch (chainSettings.lowCutSlope)
	{
	case Slope_12:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		leftLowCut.setBypassed<0>(false);
		break;
	}
	case Slope_24:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		break;
	}
	case Slope_36:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<1>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<2>().coefficients = *cutCoefficients[2];
		rightLowCut.setBypassed<2>(false);
		break;
	}
	case Slope_48:
	{
		*rightLowCut.get<0>().coefficients = *cutCoefficients[0];
		rightLowCut.setBypassed<0>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[1];
		rightLowCut.setBypassed<1>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[2];
		rightLowCut.setBypassed<2>(false);
		*rightLowCut.get<0>().coefficients = *cutCoefficients[3];
		rightLowCut.setBypassed<3>(false);
		break;
	}
	}

	juce::dsp::AudioBlock<float> block(buffer);
	auto leftBlock = block.getSingleChannelBlock(0);
	auto rightBlock = block.getSingleChannelBlock(1);

	juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
	juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

	leftChain.process(leftContext);
	rightChain.process(rightContext);
}

//==============================================================================
bool SimpleEqAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEqAudioProcessor::createEditor()
{
	//return new SimpleEqAudioProcessorEditor (*this);
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void SimpleEqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
	ChainSettings settings;

	settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
	settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
	settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
	settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
	settings.lowCutFreq = apvts.getRawParameterValue("Peak Quality")->load();
	settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
	settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

	return settings;
}

void SimpleEqAudioProcessor::updatePeakFilter(const ChainSettings chainSettings)
{
	
	auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
		chainSettings.peakFreq,
		chainSettings.peakQuality,
		juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));


	updateCoefficients(leftChain.get<chainPositions::Peak>().coefficients, *peakCoefficients);
	updateCoefficients(rightChain.get<chainPositions::Peak>().coefficients, *peakCoefficients);
}

void SimpleEqAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
	*old = *replacements;
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEqAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq", juce::NormalisableRange<float
	>(20.f, 20000.f, 1.f, 1.f), 20.f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float
	>(20.f, 20000.f, 1.f, 1.f), 20000.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq", juce::NormalisableRange<float
	>(20.f, 20000.0f, 1.f, 1.f), 750.f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain", juce::NormalisableRange<float
	>(-24.f, 24.f, 0.5f, 1.0f), 0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality", "Peak Quality", juce::NormalisableRange<float
	>(0.1f, 10.f, 0.05f, 1.f), 1.f));


	juce::StringArray stringArray;
	for (int i = 0; i < 4; ++i)
	{
		juce::String str;
		str << (12 + i * 12);
		str << " dB/Oct";
		stringArray.add(str);
	}

	layout.add(std::make_unique < juce::AudioParameterChoice >("LowCut Slope", "LowCut Slope", stringArray, 0));
	layout.add(std::make_unique < juce::AudioParameterChoice >("HighCut Slope", "HighCut Slope", stringArray, 0));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new SimpleEqAudioProcessor();
}
