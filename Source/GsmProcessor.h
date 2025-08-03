#pragma once

#include <memory>
#include <JuceHeader.h>
#include "Utilities.h"

extern "C" {
#include "gsm/config.h"
#include "gsm/gsm.h"
#include "gsm/private.h"
#include "gsm/proto.h"
#include "gsm/unproto.h"
}

//==============================================================================
class GSMProcessor : public CodecProcessorBase
{
    std::unique_ptr<gsm_state> encode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_state> decode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_signal[]> gsmSignalInput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> gsmSignal = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> gsmSignalOutput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_byte[]> gsmFrame = std::make_unique<gsm_byte[]>(33);
    
    CodecProcessorParameters parameters;
    
    float sampleRate = 44100;
    int gsmSignalCounter = 0;
    int downsamplingCounter = 0;
    int resamplingFilterOrder = 8;
    
    float currentSample = 0.0f;
    
    using IIR = juce::dsp::IIR::Filter<float>;
    IIR lowCutFilter;
    std::vector<IIR> preFilters;
    std::vector<IIR> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> filterCoefficientsArray;
    
public:
    GSMProcessor();
    
    ~GSMProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
};
