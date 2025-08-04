#include "VoxProcessor.h"
#include "Utilities.h"
#include "juce_dsp/juce_dsp.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
// #include <cstdint>

VoxProcessor::VoxProcessor() = default;

VoxProcessor::~VoxProcessor() = default;

void VoxProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<int>(spec.sampleRate);
    int numChannels = static_cast<int>(spec.numChannels);
    
    auto lowCutCoefficients = dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 20.0f);
    
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / parameters.downsampling) * 0.4, sampleRate, resamplingFilterOrder);
    
    preLowCutFilter.resize(numChannels);
    postLowCutFilter.resize(numChannels);
    preFilters.resize(numChannels);
    postFilters.resize(numChannels);
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        preFilters[channel].resize(resamplingFilterOrder / 2);
        postFilters[channel].resize(resamplingFilterOrder / 2);
        
        preLowCutFilter[channel].reset();
        preLowCutFilter[channel].prepare(spec);
        preLowCutFilter[channel].coefficients = lowCutCoefficients;
        
        postLowCutFilter[channel].reset();
        postLowCutFilter[channel].prepare(spec);
        postLowCutFilter[channel].coefficients = lowCutCoefficients;
        
        for (int filter = 0; filter < resamplingFilterOrder / 2; ++filter)
        {
            // prepare each pre-filter
            preFilters[channel][filter].reset();
            preFilters[channel][filter].prepare(spec);
            preFilters[channel][filter].coefficients = filterCoefficientsArray.getObjectPointer(filter);
            
            // prepare each post-filter
            postFilters[channel][filter].reset();
            postFilters[channel][filter].prepare(spec);
            postFilters[channel][filter].coefficients = filterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    reset();
}

void VoxProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = preLowCutFilter[channel].processSample(channelData[sample]);
            preLowCutFilter[channel].snapToZero();
            // scale -1–1 to 16-bit int range
            int16_t pcmIn = static_cast<int16_t>(channelData[sample] * ((1 << 12) - 1));
            uint8_t compressed = voxEncode(pcmIn);
            // if noise gate closed, alternate +/- 0
            // compressed = VOX_RESET_TABLE[sample %= 2];
            // if (resetCounter < 48) {
            //     compressed = VOX_RESET_TABLE[resetCounter %= 2];
            //     resetCounter += 1;
            // }
            int16_t pcmOut = voxDecode(compressed);
            channelData[sample] = static_cast<float>(pcmOut) / ((1 << 15) - 1);
            channelData[sample] = postLowCutFilter[channel].processSample(channelData[sample]);
            postLowCutFilter[channel].snapToZero();
        }
    }
}

void VoxProcessor::reset() {}

CodecProcessorParameters& VoxProcessor::getParameters() { return parameters; }

void VoxProcessor::setParameters(const CodecProcessorParameters& params)
{
    // coefficients
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / params.downsampling) * 0.4, sampleRate, resamplingFilterOrder);
    
    for (int channel = 0; channel < preFilters.size(); ++channel)
    {
        for (int filter = 0; filter < resamplingFilterOrder / 2; ++filter)
        {
            // update each pre-filter
            preFilters[channel][filter].coefficients = filterCoefficientsArray.getObjectPointer(filter);
            
            // update each post-filter
            postFilters[channel][filter].coefficients = filterCoefficientsArray.getObjectPointer(filter);
        }
    }

    parameters = params;
}

uint8_t VoxProcessor::voxEncode(int16_t& inSample) {
    // calculate differece btwn last time/this; divide by 16 because we're working at 12
    // bits
    int16_t diff = (inSample / 16) - encodeState.predictor;
    // step size for this time
    int16_t stepSize = VOX_STEP_TABLE[encodeState.stepIndex];
    // step index to use for next time
    int16_t stepIndex = encodeState.stepIndex + ADPCM_INDEX_TABLE[encodeState.outSample];
    stepIndex = std::clamp<int16_t>(stepIndex, 0, static_cast<int16_t>(sizeof(VOX_STEP_TABLE)/sizeof(VOX_STEP_TABLE[0]) - 1));
    
    // encoder block based on pseudocode in spec
    uint8_t bits = 0b0000;
    if (diff < 0) { 
        bits |= 0b1000; 
    }
    diff = std::abs<int16_t>(diff);
    if (diff >= stepSize) {
        bits |= 0b0100;
        diff -= stepSize;
    }
    if (diff >= (stepSize >> 1)) {
        bits |= 0b0010;
        diff -= (stepSize >> 1);
    }
    if (diff >= (stepSize >> 2)) {
        bits |= 0b0001;
    }
    
    // decode block from self.vox_decode, NOT full function
    // sign is 4th bit; magnitude is 3 LSBs
    uint8_t sign = bits & 0b1000;
    uint8_t magnitude = bits & 0b0111;
    // calculate difference based on pseudocode in spec
    int16_t delta = ((2 * static_cast<int16_t>(magnitude) + 1) * stepSize) >> 3;
    // last time's value
    int16_t predictor = encodeState.predictor;
    // if sign bit (4th one) is set, value is negative
    if (sign != 0) {
        delta *= -1;
    }
    predictor *= delta;
    
    // push values into z^-1 delays
    encodeState.predictor = std::clamp<int16_t>(predictor, -(1 << 11), (1 << 11) - 1);
    
    encodeState.stepIndex = stepIndex;
    encodeState.outSample = bits;
    
    // return vox sample
    return bits;
}

int16_t VoxProcessor::voxDecode(uint8_t& inNibble) {
    // get step size from last time's index before updating
    int16_t stepSize = VOX_STEP_TABLE[decodeState.stepIndex];
    // use in_nibble to index into adpcm step table; add to step
    int16_t stepIndex = decodeState.stepIndex + ADPCM_INDEX_TABLE[inNibble];
    // clamp index to size of step table — for next time
    stepIndex = std::clamp<int16_t>(stepIndex, 0, static_cast<int16_t>(sizeof(VOX_STEP_TABLE)/sizeof(VOX_STEP_TABLE[0]) - 1));
    
    // sign is 4th bit; magnitude is 3 LSBs
    uint8_t sign = inNibble & 0b1000;
    uint8_t magnitude = inNibble & 0b0111;
    
    // magnitude; after * 2 and >> 3, equivalent to scale of 3 bits in (ss(n)*B2)+(ss(n)/2*B1)+(ss(n)/4*BO) from pseudocode
    // + 1: after >> 3, corresponds to ss(n)/8 from pseudocode — bit always multiplies step, regardless of 3 magnitude bits on/off
    int16_t delta = ((2 * static_cast<int16_t>(magnitude) + 1) * stepSize) >> 3;
    // last time's value
    int16_t predictor = decodeState.predictor;
    // if sign bit (4th one) is set, value is negative
    if (sign != 0) {
        delta *= -1;
    }
    predictor += delta;
    
    // clamp output between 12-bit signed min/max value
    decodeState.predictor = std::clamp<int16_t>(predictor, -(1 << 11), (1 << 11) - 1);
    // update for next time through; ss(n+1) into z-1 from block diagram
    decodeState.stepIndex = stepIndex;
    // return updated predictor, which is also saved for next time; X(n) into z-1
    // scale from 12-bit to 16-bit; 16 = 2^4, or 4 extra bits
    return decodeState.predictor * 16;
}
