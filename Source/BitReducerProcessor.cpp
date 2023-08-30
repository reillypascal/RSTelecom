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
            float input = 0.0f;
            
            if (parameters.downsampling > 1)
            {
                // pre-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = preFilters[channel][filter].processSample(channelData[sample]);
                    preFilters[channel][filter].snapToZero();
                }
                
                if (mDownsamplingCounter[channel] == 0)
                    input = channelData[sample];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    input = postFilters[channel][filter].processSample(input);
                    postFilters[channel][filter].snapToZero();
                }
            }
            else
                input = channelData[sample];
            
            std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
            input *= downsamplingGainComp[parameters.downsampling];
            
            int16_t pcm_in = static_cast<int16_t>(input * 32767.0);
            uint8_t compressed = Lin2MuLaw(pcm_in);
            
            int16_t pcm_out = MuLaw2Lin(compressed);
            channelData[sample] = static_cast<float>(pcm_out) * mOutScale;
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
    int16_t mask;
    int16_t seg;
    uint8_t uval;
    
    pcm_val = pcm_val >> 2;
    if (pcm_val < 0)
    {
        pcm_val = -pcm_val;
        mask = 0x7f;
    }
    else
    {
        mask = 0xff;
    }
    
    if (pcm_val > 8159) pcm_val = 8159;
    pcm_val += (0x84 >> 2);
    
    if (pcm_val <= 0x3f) seg = 0;
    else if (pcm_val <= 0x7f) seg = 1;
    else if (pcm_val <= 0xff) seg = 2;
    else if (pcm_val <= 0x1ff) seg = 3;
    else if (pcm_val <= 0x3ff) seg = 4;
    else if (pcm_val <= 0x7ff) seg = 5;
    else if (pcm_val <= 0xfff) seg = 6;
    else if (pcm_val <= 0x1fff) seg = 7;
    else seg = 8;
    
    if (seg >= 8)
        return static_cast<uint8_t>(0x7f ^ mask);
    else
    {
        uval = static_cast<uint8_t>((seg << 4) | ((pcm_val >> (seg + 1)) & 0x0f));
        return (uval ^ mask);
    }
}

inline short MuLawProcessor::MuLaw2Lin(uint8_t u_val)
{
    int16_t t;
    u_val = ~u_val;
    t = ((u_val & 0x0f) << 3) + 0x84;
    t <<= (static_cast<unsigned>(u_val) & 0x70) >> 4;
    return ((u_val & 0x80) ? (0x84 - t) : (t - 0x84));
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

void ALawProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = 0.0f;
            
            // choose downsampling or no
            if (parameters.downsampling > 1)
            {
                // pre-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    channelData[sample] = preFilters[channel][filter].processSample(channelData[sample]);
                    preFilters[channel][filter].snapToZero();
                }
                
                // downsampling
                if (mDownsamplingCounter[channel] == 0)
                    input = channelData[sample];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-filtering
                for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
                {
                    input = postFilters[channel][filter].processSample(input);
                    postFilters[channel][filter].snapToZero();
                }
            }
            else
                input = channelData[sample];
            
            // A-law processing
            std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
            input *= downsamplingGainComp[parameters.downsampling];
            
//            int16_t pcm_in = static_cast<int16_t>(input * 32767.0);
//            uint8_t compressed = Lin2ALaw(pcm_in);
//
//            int16_t pcm_out = ALaw2Lin(compressed);
//            channelData[sample] = static_cast<float>(pcm_out) * mOutScale;
            channelData[sample] = input;
        }
    }
}

void ALawProcessor::reset() {}

CodecProcessorParameters& ALawProcessor::getParameters() { return parameters; }

void ALawProcessor::setParameters(const CodecProcessorParameters &params)
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
