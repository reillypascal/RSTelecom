#pragma once

#include "Utilities.h"
#include "juce_dsp/juce_dsp.h"
#include <JuceHeader.h>
#include <cstddef>
#include <cstdint>

struct VoxState {
    int16_t predictor = 0;
    int16_t stepIndex = 0;
    uint8_t outSample = 0;
};

// ============================

class VoxCodec {
public:
    VoxCodec();
    
    ~VoxCodec();
    
    void reset();
    
    uint8_t voxEncode(int16_t& inSample);
    
    int16_t voxDecode(uint8_t& inNibble);
private:
    // “Array bound cannot be deduced from a default member initializer” if just const 
    static constexpr int16_t ADPCM_INDEX_TABLE[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
    
    static constexpr int16_t VOX_STEP_TABLE[] = { 
        16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552,
    };
    
    VoxState encodeState;
    VoxState decodeState;
};

// =================================

class VoxProcessor : public CodecProcessorBase
{
public:
    VoxProcessor();
    
    ~VoxProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
    
private:
    // uint8_t voxEncode(int16_t& inSample, VoxState& state);
    
    // int16_t voxDecode(uint8_t& inNibble, VoxState& state);
    // // “Array bound cannot be deduced from a default member initializer” if just const 
    // static constexpr int16_t ADPCM_INDEX_TABLE[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 };
    
    // static constexpr int16_t VOX_STEP_TABLE[] = { 
    //     16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
    //     143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    //     876, 963, 1060, 1166, 1282, 1411, 1552,
    // };
    // // +/- 0 for resetting 
    // static constexpr uint8_t VOX_RESET_TABLE[] = { 0b1000, 0b0000 };
    // int resetCounter = 0;
    
    std::vector<VoxCodec> vox;
    
    CodecProcessorParameters parameters;
    
    float sampleRate = 44100;
    int resamplingFilterOrder = 8;
    std::vector<int> downsamplingCounter { 0, 0 };
    std::vector<float> downsamplingInput { 0.0f, 0.0f };
    
    using IIR = juce::dsp::IIR::Filter<float>;
    std::vector<IIR> postLowCutFilter;
    std::vector<std::vector<IIR>> preFilters;
    std::vector<std::vector<IIR>> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> filterCoefficientsArray;
};
