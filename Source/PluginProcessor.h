/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BitReducerProcessor.h"
#include "CodecProcessor.h"

struct ProcessorFactory
{
    std::unique_ptr<CodecProcessorBase> create(int type)
    {
        auto iter = processorMapping.find(type);
        if (iter != processorMapping.end())
            return iter->second();
        
        return nullptr;
    }
    
    std::map<
        int,
        std::function<std::unique_ptr<CodecProcessorBase>()>> processorMapping
    {
        { 0, []() { return nullptr; } },
        { 1, []() { return std::make_unique<GSMProcessor>(); } },
        { 3, []() { return std::make_unique<MuLawProcessor>(); } }
    };
};


//==============================================================================
/**
*/
class RSTelecomAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    RSTelecomAudioProcessor();
    ~RSTelecomAudioProcessor() override;

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

private:
    juce::AudioProcessorValueTreeState parameters;
    
    juce::AudioParameterChoice* downsamplingParameter = nullptr;
    juce::AudioParameterChoice* bitrateParameter = nullptr;
    std::atomic<float>* saturationParameter = nullptr;
    
    std::atomic<float>* errorClockParameter = nullptr;
    std::atomic<float>* errorProbParameter = nullptr;
    
    juce::AudioParameterChoice* slot1MenuParameter = nullptr;
    juce::AudioParameterChoice* slot2MenuParameter = nullptr;
    
//    GSMProcessor gsmProcessor;
//    
//    MuLawProcessor muLawProcessor;
    
    ProcessorFactory processorFactory {};
    
    std::unique_ptr<CodecProcessorBase> slot1Processor = nullptr;
    std::unique_ptr<CodecProcessorBase> slot2Processor = nullptr;
    
    int prevSlot1Codec = -1;
    int prevSlot2Codec = -1;
    
    CodecProcessorParameters processorParameters;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSTelecomAudioProcessor)
};
