#include "BitReducerProcessor.h"

MuLawProcessor::MuLawProcessor() = default;

MuLawProcessor::~MuLawProcessor() = default;

void MuLawProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    
    // coefficients
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, 8);
    
    auto numChannels = spec.numChannels;
    
    preFilter1.resize(numChannels);
    for (auto& ch : preFilter1)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
    }
    preFilter2.resize(numChannels);
    for (auto& ch : preFilter2)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
    }
    preFilter3.resize(numChannels);
    for (auto& ch : preFilter3)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
    }
    preFilter4.resize(numChannels);
    for (auto& ch : preFilter4)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
    }
    
    postFilter1.resize(numChannels);
    for (auto& ch : postFilter1)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
    }
    postFilter2.resize(numChannels);
    for (auto& ch : postFilter2)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
    }
    postFilter3.resize(numChannels);
    for (auto& ch : postFilter3)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
    }
    postFilter4.resize(numChannels);
    for (auto& ch : postFilter4)
    {
        ch.reset();
        ch.prepare(spec);
        ch.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
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
                // pre-downsampling filters
                channelData[sample] = preFilter1[channel].processSample(channelData[sample]);
                preFilter1[channel].snapToZero();
                channelData[sample] = preFilter2[channel].processSample(channelData[sample]);
                preFilter2[channel].snapToZero();
                channelData[sample] = preFilter3[channel].processSample(channelData[sample]);
                preFilter3[channel].snapToZero();
                channelData[sample] = preFilter4[channel].processSample(channelData[sample]);
                preFilter4[channel].snapToZero();
                
                if (mDownsamplingCounter[channel] == 0)
                    input = channelData[sample];
                
                ++mDownsamplingCounter[channel];
                mDownsamplingCounter[channel] %= parameters.downsampling;
                
                // post-downsampling images filters
                input = postFilter1[channel].processSample(input);
                postFilter1[channel].snapToZero();
                input = postFilter2[channel].processSample(input);
                postFilter2[channel].snapToZero();
                input = postFilter3[channel].processSample(input);
                postFilter3[channel].snapToZero();
                input = postFilter4[channel].processSample(input);
                postFilter4[channel].snapToZero();
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
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, 8);
        
        for (auto& ch : preFilter1)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
        
        for (auto& ch : preFilter2)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
        
        for (auto& ch : preFilter3)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
        
        for (auto& ch : preFilter4)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
        
        
        for (auto& ch : postFilter1)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
        
        for (auto& ch : postFilter2)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
        
        for (auto& ch : postFilter3)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
        
        for (auto& ch : postFilter4)
            ch.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
        
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
