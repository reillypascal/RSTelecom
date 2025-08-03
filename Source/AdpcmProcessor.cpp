#include "AdpcmProcessor.h"
#include "Utilities.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
// #include <cstdint>

VoxProcessor::VoxProcessor() = default;

VoxProcessor::~VoxProcessor() = default;

void VoxProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<int>(spec.sampleRate);
    numChannels = static_cast<int>(spec.numChannels);
    
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / parameters.downsampling) * 0.4, sampleRate, resamplingFilterOrder);
    
    preFilters.resize(numChannels);
    postFilters.resize(numChannels);
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        preFilters[channel].resize(resamplingFilterOrder / 2);
        postFilters[channel].resize(resamplingFilterOrder / 2);
        
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
    
}

void VoxProcessor::reset() {}

CodecProcessorParameters& VoxProcessor::getParameters() { return parameters; }

void VoxProcessor::setParameters(const CodecProcessorParameters& params)
{
    // coefficients
    filterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((sampleRate / params.downsampling) * 0.4, sampleRate, resamplingFilterOrder);
    
    for (int channel = 0; channel < numChannels; ++channel)
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

unsigned char VoxProcessor::voxEncode(int16_t& inSample) {
    int16_t diff = (inSample / 16) - encodeState.predictor;
    int16_t stepSize = VOX_STEP_TABLE[encodeState.stepIndex];
    int16_t stepIndex = encodeState.stepIndex + ADPCM_INDEX_TABLE[encodeState.outSample];
    
    stepIndex = std::clamp(stepIndex, int16_t{0}, static_cast<int16_t>(sizeof(VOX_STEP_TABLE)/sizeof(VOX_STEP_TABLE[0]) - 1));
    
    unsigned char bits = 0b0000;
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
    
    unsigned char sign = bits & 0b1000;
    unsigned char magnitude = bits & 0b0111;
    
    int16_t delta = (2 * static_cast<int16_t>(magnitude) * stepSize) >> 3;
    
    int16_t predictor = encodeState.predictor;
    
    if (sign != 0) {
        delta *= -1;
    }
    predictor *= delta;
    
    encodeState.predictor = std::clamp<int16_t>(predictor, -(1 << 11), (1 << 11) - 1);
    
    encodeState.stepIndex = stepIndex;
    encodeState.outSample = bits;
    
    return bits;
}

int16_t VoxProcessor::voxDecode(unsigned char& inNibble) {
    // get step size from last time's index before updating
    int16_t stepSize = VOX_STEP_TABLE[decodeState.stepIndex];
    // use in_nibble to index into adpcm step table; add to step
    int16_t stepIndex = decodeState.stepIndex + ADPCM_INDEX_TABLE[encodeState.outSample];
    // clamp index to size of step table — for next time
    stepIndex = std::clamp<int16_t>(stepIndex, 0, static_cast<int16_t>(sizeof(VOX_STEP_TABLE)/sizeof(VOX_STEP_TABLE[0]) - 1));
    
    // sign is 4th bit; magnitude is 3 LSBs
    unsigned char sign = inNibble & 0b1000;
    unsigned char magnitude = inNibble & 0b0111;
    
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
