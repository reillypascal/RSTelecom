/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RSTelecomAudioProcessor::RSTelecomAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, juce::Identifier("RSTelecom"), {
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID
                                                    { "downsampling", 1 },
                                                    "Downsampling",
                                                     juce::StringArray { "1x", "2x", "3x", "4x", "5x", "6x", "7x", "8x" },
                                                     0),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID
                                                     { "bitrate", 1 },
                                                     "Bitrate",
                                                     juce::StringArray { "Default", "2x", "1/2", "1/4" },
                                                     0),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID
                                                    { "saturation", 1 },
                                                    "Saturation",
                                                    0.0f,
                                                    24.0f,
                                                    0.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID
                                                    { "errorClock", 1 },
                                                    "Error Clock",
                                                    20.0f,
                                                    3500.0f,
                                                    1500.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID
                                                    { "errorProb", 1 },
                                                    "Error Probability",
                                                    0.0f,
                                                    1.0f,
                                                    0.0f),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID
                                                     { "slot1", 1 },
                                                     "Slot 1",
                                                     juce::StringArray { "None", "GSM 06.10", "iLBC", "Mu-Law", "A-Law" },
                                                     0),
        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID
                                                     { "slot2", 1 },
                                                     "Slot 2",
                                                     juce::StringArray { "None", "GSM 06.10", "iLBC", "Mu-Law", "A-Law" },
                                                     0)
    })
{
    downsamplingParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("downsampling"));
    bitrateParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("bitrate"));
    saturationParameter = parameters.getRawParameterValue("saturation");
    
    errorClockParameter = parameters.getRawParameterValue("errorClock");
    errorProbParameter = parameters.getRawParameterValue("errorProb");
    
    slot1MenuParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("slot1"));
    slot2MenuParameter = static_cast<juce::AudioParameterChoice*>(parameters.getParameter("slot2"));
}

RSTelecomAudioProcessor::~RSTelecomAudioProcessor()
{
}

//==============================================================================
const juce::String RSTelecomAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RSTelecomAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RSTelecomAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RSTelecomAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RSTelecomAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RSTelecomAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RSTelecomAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RSTelecomAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RSTelecomAudioProcessor::getProgramName (int index)
{
    return {};
}

void RSTelecomAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RSTelecomAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void RSTelecomAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RSTelecomAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RSTelecomAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    slotCodecs[0] = slot1MenuParameter->getIndex();
    slotCodecs[1] = slot2MenuParameter->getIndex();
    
    // create and update processors
    for (int i = 0; i < mNumProcessorSlots; ++i)
    {
        if (slotCodecs[i] != prevSlotCodecs[i])
        {
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = getSampleRate();
            spec.maximumBlockSize = buffer.getNumSamples();
            spec.numChannels = buffer.getNumChannels();
            
            slotProcessors[i] = processorFactory.create(slotCodecs[i]);
            
            if (slotProcessors[i] != nullptr)
                slotProcessors[i]->prepare(spec);
            
            prevSlotCodecs[i] = slotCodecs[i];
        }
    }
    
    // update parameters, process audio
    for (int i = 0; i < mNumProcessorSlots; ++i)
    {
        if (slotProcessors[i] != nullptr)
        {
            processorParameters = slotProcessors[i]->getParameters();
            processorParameters.downsampling = downsamplingParameter->getIndex() + 1;
            processorParameters.bitrate = bitrateParameter->getIndex() + 1;
            
            slotProcessors[i]->setParameters(processorParameters);
            
            slotProcessors[i]->processBlock(buffer, midiMessages);
        }
    }
    
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//
//    }
}

//==============================================================================
bool RSTelecomAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RSTelecomAudioProcessor::createEditor()
{
    return new RSTelecomAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RSTelecomAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RSTelecomAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RSTelecomAudioProcessor();
}
