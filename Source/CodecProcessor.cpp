#include "CodecProcessor.h"


//==============================================================================
GSMProcessor::GSMProcessor() = default;

GSMProcessor::~GSMProcessor() = default;

void GSMProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, mResamplingFilterOrder);
    
    preFilters.resize(mResamplingFilterOrder / 2);
    postFilters.resize(mResamplingFilterOrder / 2);
    
    for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
    {
        // prepare each pre-filter
        preFilters[filter].reset();
        preFilters[filter].prepare(spec);
        preFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        
        // prepare each post-filter
        postFilters[filter].reset();
        postFilters[filter].prepare(spec);
        postFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
    }
    
    reset();
}

void GSMProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    juce::AudioBuffer<float> monoBuffer(1, numSamples);
    monoBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);
    if (numChannels > 1)
    {
        monoBuffer.addFrom(0, 0, buffer, 1, 0, numSamples);
        monoBuffer.applyGain(0.5f);
    }
    
    auto* src = monoBuffer.getWritePointer(0);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // if gsm data variables are valid, process audio
        if (mGsmSignalInput != nullptr && mGsmSignal != nullptr && mGsmSignalOutput != nullptr)
        {
            // low cut filter
            src[sample] = lowCutFilter.processSample(src[sample]);
            
            // pre-filtering
            for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
            {
                src[sample] = preFilters[filter].processSample(src[sample]);
                preFilters[filter].snapToZero();
            }
            
            // move filter gain compensation before gsm to minimize noise?
            
            // prepare to store next sample
            float currentSample { 0.0f };
            // sync data rate to downsampling counter
            if (mDownsamplingCounter == 0)
            {
                mGsmSignalInput.get()[mGsmSignalCounter] = static_cast<gsm_signal>(src[sample] * 4096.0f);

                mGsmSignalInput.get()[mGsmSignalCounter] <<= 3;
                mGsmSignalInput.get()[mGsmSignalCounter] &= 0b1111111111111000;
                
                // increment gsm frame
                ++mGsmSignalCounter;
                mGsmSignalCounter %= 160;
                
                // sync data rate to gsm frame
                if (mGsmSignalCounter == 0)
                {
                    std::swap(mGsmSignalInput, mGsmSignal);
                    gsm_encode(mEncode.get(), mGsmSignal.get(), mGsmFrame.get());
                    gsm_decode(mDecode.get(), mGsmFrame.get(), mGsmSignal.get());
                    std::swap(mGsmSignal, mGsmSignalOutput);
                }

                mGsmSignalOutput.get()[mGsmSignalCounter] >>= 3;
                currentSample = static_cast<float>(mGsmSignalOutput.get()[mGsmSignalCounter]) / 4096.0f;
            }
            
            // increment downsampling frame
            ++mDownsamplingCounter;
            mDownsamplingCounter %= parameters.downsampling;
            
            // post-filtering
            for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
            {
                currentSample = postFilters[filter].processSample(currentSample);
                postFilters[filter].snapToZero();
            }
            
            // filter gain compensation - move before gsm to minimize noise?
            std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
            src[sample] *= downsamplingGainComp[parameters.downsampling];
            
            // move from mono gsm signal to stereo out buffer
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* dst = buffer.getWritePointer(channel);
                dst[sample] = currentSample;
            }
        }
    }
}

void GSMProcessor::reset() {}

CodecProcessorParameters& GSMProcessor::getParameters() { return parameters; }

void GSMProcessor::setParameters(const CodecProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        // coefficients
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / params.downsampling) * 0.4, mSampleRate, 8);
        
        for (int filter = 0; filter < mResamplingFilterOrder / 2; ++filter)
        {
            // update each pre-filter
            preFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
            
            // update each post-filter
            postFilters[filter].coefficients = mFilterCoefficientsArray.getObjectPointer(filter);
        }
    }
    
    parameters = params;
}
