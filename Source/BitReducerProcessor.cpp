#include "BitReducerProcessor.h"

MuLawProcessor::MuLawProcessor() = default;

MuLawProcessor::~MuLawProcessor() = default;

void MuLawProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    mNumChannels = spec.numChannels;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
    
    preFilters.resize(mNumChannels);
    postFilters.resize(mNumChannels);
    
    for (int channel = 0; channel < mNumChannels; ++channel)
    {
        preFilters[channel].resize(mResamplingFilterOrder / 2);
        postFilters[channel].resize(mResamplingFilterOrder / 2);
        
        for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
        {
            // prepare each pre-filter
            preFilters[channel][filter].reset();
            preFilters[channel][filter].prepare(spec);
            preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            
            // prepare each post-filter
            postFilters[channel][filter].reset();
            postFilters[channel][filter].prepare(spec);
            postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    reset();
}

void MuLawProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Mu-Law processing on channelData
            int16_t pcm_in = static_cast<int16_t>(channelData[sample] * 32767.0);
            uint8_t compressed = Lin2MuLaw(pcm_in);
            
            int16_t pcm_out = MuLaw2Lin(compressed);
            channelData[sample] = static_cast<float>(pcm_out) * mOutScale;
            
            // downsample and filter
            if (parameters.downsampling > 1)
            {
                // pre-filtering; write from/to channelData
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = preFilters[channel][filter].processSample(channelData[sample]);
                    preFilters[channel][filter].snapToZero();
                }
                
                // downsampling; increment input sample from channelData
                if (mDownsamplingCounter[channel] == 0)
                    mDownsamplingInput[channel] = channelData[sample];
                
                channelData[sample] = mDownsamplingInput[channel];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-filtering; take in input sample, write to channelData
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = postFilters[channel][filter].processSample(channelData[sample]);
                    postFilters[channel][filter].snapToZero();
                }
                
//                channelData[sample] *= 1.0f + ((parameters.downsampling - 1.0f) * 0.08);
            }
        }
    }
}

void MuLawProcessor::reset() {}

CodecProcessorParameters& MuLawProcessor::getParameters() { return parameters; }

void MuLawProcessor::setParameters(const CodecProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        // coefficients
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / params.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
        
        for (int channel = 0; channel < mNumChannels; ++channel)
        {
            for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
            {
                // update each pre-filter
                preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
                
                // update each post-filter
                postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            }
        }
    }
    
    parameters = params;
}

inline unsigned char MuLawProcessor::Lin2MuLaw(int16_t pcm_val)
{
    int sign = (pcm_val >> 8) & 0x80;
    if (sign)
        pcm_val = static_cast<int16_t>(-pcm_val);
    if (pcm_val > cClip)
        pcm_val = cClip;
    pcm_val = static_cast<int16_t>(pcm_val + cBias);
    int exponent = static_cast<int>(MuLawCompressTable[(pcm_val >> 7) & 0xff]);
    int mantissa = (pcm_val >> (exponent + 3)) & 0x0f;
    int compressedByte = ~(sign | (exponent << 4) | mantissa);
    
    return static_cast<unsigned char>(compressedByte);
}

inline short MuLawProcessor::MuLaw2Lin(uint8_t u_val)
{
    return MuLawDecompressTable[u_val];
}


//=======================================================================

ALawProcessor::ALawProcessor() = default;

ALawProcessor::~ALawProcessor() = default;

void ALawProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    mNumChannels = spec.numChannels;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
    
    preFilters.resize(mNumChannels);
    postFilters.resize(mNumChannels);
    
    for (int channel = 0; channel < mNumChannels; ++channel)
    {
        preFilters[channel].resize(mResamplingFilterOrder / 2);
        postFilters[channel].resize(mResamplingFilterOrder / 2);
        
        for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
        {
            // prepare each pre-filter
            preFilters[channel][filter].reset();
            preFilters[channel][filter].prepare(spec);
            preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            
            // prepare each post-filter
            postFilters[channel][filter].reset();
            postFilters[channel][filter].prepare(spec);
            postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    reset();
}

void ALawProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // A-law processing on channelData
            int16_t pcm_in = static_cast<int16_t>(channelData[sample] * 32767.0);
            uint8_t compressed = Lin2ALaw(pcm_in);

            int16_t pcm_out = ALaw2Lin(compressed);
            channelData[sample] = static_cast<float>(pcm_out) * mOutScale;
            
            // downsample and filter
            if (parameters.downsampling > 1)
            {
                // pre-filtering; write from/to channelData
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = preFilters[channel][filter].processSample(channelData[sample]);
                    preFilters[channel][filter].snapToZero();
                }
                
                // downsampling; increment input sample from channelData
                if (mDownsamplingCounter[channel] == 0)
                    mDownsamplingInput[channel] = channelData[sample];
                
                channelData[sample] = mDownsamplingInput[channel];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-filtering; take in input sample, write to channelData
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = postFilters[channel][filter].processSample(channelData[sample]);
                    postFilters[channel][filter].snapToZero();
                }
                
//                channelData[sample] *= 1.0f + ((parameters.downsampling - 1.0f) * 0.08);
            }
        }
    }
}

void ALawProcessor::reset() {}

CodecProcessorParameters& ALawProcessor::getParameters() { return parameters; }

void ALawProcessor::setParameters(const CodecProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / params.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
        
        for (int channel = 0; channel < mNumChannels; ++channel)
        {
            for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
            {
                // update each pre-filter
                preFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
                
                // update each post-filter
                postFilters[channel][filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            }
        }
    }
    
    parameters = params;
}

unsigned char ALawProcessor::Lin2ALaw(int16_t pcm_val)
{
    int sign;
    int exponent;
    int mantissa;
    unsigned char compressedByte;
    
    sign = ((~pcm_val) >> 8) & 0x80;
    if (!sign)
        pcm_val = static_cast<int16_t>(-pcm_val);
    if (pcm_val > cClip)
        pcm_val = cClip;
    if (pcm_val >= 256)
    {
        exponent = static_cast<int>(ALawCompressTable[(pcm_val >> 8) & 0x7f]);
        mantissa = (pcm_val >> (exponent + 3)) & 0x0f;
        compressedByte = ((exponent << 4) | mantissa);
    }
    else
        compressedByte = static_cast<unsigned char>(pcm_val >> 4);
    
    compressedByte ^= (sign ^ 0x55);
    return compressedByte;
}

short ALawProcessor::ALaw2Lin(uint8_t a_val)
{
    return ALawDecompressTable[a_val];
}
