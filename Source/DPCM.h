#pragma once

#include <JuceHeader.h>
// #include <cstddef>
#include <cstdint>
#include "Utilities.h"

class DPCMProcessor : public CodecProcessorBase
{
public:
    DPCMProcessor();
    
    ~DPCMProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
    
private:
    uint8_t dpcmEncoder(float inVal);
    
    float dpcmDecoder(uint8_t inVal);
};
