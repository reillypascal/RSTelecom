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
    std::unique_ptr<gsm_state> mEncode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_state> mDecode = std::make_unique<gsm_state>();
    std::unique_ptr<gsm_signal[]> mGsmSignalInput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> mGsmSignal = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_signal[]> mGsmSignalOutput = std::make_unique<gsm_signal[]>(160);
    std::unique_ptr<gsm_byte[]> mGsmFrame = std::make_unique<gsm_byte[]>(33);
    
    CodecProcessorParameters parameters;
    int mSampleRate { 44100 };
    int mGsmSignalCounter { 0 };
    int mDownsamplingCounter { 0 };
    int mResamplingFilterOrder { 8 };
    
    float mCurrentSample { 0.0f };
        
    using IIR = juce::dsp::IIR::Filter<float>;
    IIR lowCutFilter;
    std::vector<IIR> preFilters;
    std::vector<IIR> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
    
public:
    GSMProcessor();
    
    ~GSMProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
};

//==============================================================================
// opus
