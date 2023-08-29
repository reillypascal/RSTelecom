#include "CodecProcessor.h"


//==============================================================================
GSMProcessor::GSMProcessor() = default;

GSMProcessor::~GSMProcessor() = default;

void GSMProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = spec.sampleRate;
    
    mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, 8);
    
    // pre filters
    preFilter1.reset();
    preFilter1.prepare(spec);
    preFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
    
    preFilter2.reset();
    preFilter2.prepare(spec);
    preFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
    
    preFilter3.reset();
    preFilter3.prepare(spec);
    preFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
    
    preFilter4.reset();
    preFilter4.prepare(spec);
    preFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
    
    // post filters
    postFilter1.reset();
    postFilter1.prepare(spec);
    postFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
    
    postFilter2.reset();
    postFilter2.prepare(spec);
    postFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
    
    postFilter3.reset();
    postFilter3.prepare(spec);
    postFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
    
    postFilter4.reset();
    postFilter4.prepare(spec);
    postFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
    
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
        // resampling filter
        src[sample] = preFilter1.processSample(src[sample]);
        preFilter1.snapToZero();
        src[sample] = preFilter2.processSample(src[sample]);
        preFilter2.snapToZero();
        src[sample] = preFilter3.processSample(src[sample]);
        preFilter3.snapToZero();
        src[sample] = preFilter4.processSample(src[sample]);
        preFilter4.snapToZero();
        
        if (mGsmSignalInput != nullptr && mGsmSignal != nullptr && mGsmSignalOutput != nullptr)
        {
            float currentSample { 0.0f };
            if (mDownsamplingCounter == 0)
            {
                mGsmSignalInput.get()[mGsmSignalCounter] = static_cast<gsm_signal>(src[sample] * 4096.0f);

                mGsmSignalInput.get()[mGsmSignalCounter] <<= 3;
                mGsmSignalInput.get()[mGsmSignalCounter] &= 0b1111111111111000;

                ++mGsmSignalCounter;
                mGsmSignalCounter %= 160;

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
            
            ++mDownsamplingCounter;
            mDownsamplingCounter %= parameters.downsampling;
            
            currentSample = postFilter1.processSample(currentSample);
            postFilter1.snapToZero();
            currentSample = postFilter2.processSample(currentSample);
            postFilter2.snapToZero();
            currentSample = postFilter3.processSample(currentSample);
            postFilter3.snapToZero();
            currentSample = postFilter4.processSample(currentSample);
            postFilter4.snapToZero();
            
            std::vector<float> downsamplingGainComp { 1.0f, 1.0f, 1.45f, 2.35f, 3.5f, 4.35f, 5.5f, 6.25f, 7.0f };
            currentSample *= downsamplingGainComp[parameters.downsampling];
            
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* dst = buffer.getWritePointer(channel);
                dst[sample] = currentSample;
            }
        }
    }
}

void GSMProcessor::reset()
{
    
}

CodecProcessorParameters& GSMProcessor::getParameters() { return parameters; }

void GSMProcessor::setParameters(const CodecProcessorParameters& params)
{
    if (parameters.downsampling != params.downsampling)
    {
        parameters.downsampling = params.downsampling;
        
        // pre filters
        preFilter1.reset();
        preFilter2.reset();
        preFilter3.reset();
        preFilter4.reset();
        
        // post filters
        postFilter1.reset();
        postFilter2.reset();
        postFilter3.reset();
        postFilter4.reset();
        
        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / parameters.downsampling) * 0.4, mSampleRate, 8);
        
        // pre filters
        preFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
        preFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
        preFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
        preFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
        
        // post filters
        postFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
        postFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
        postFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
        postFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
    }
    
    parameters = params;
}

//void GSMProcessor::setDownsampling(int newDownsampling)
//{
//    mDownsamplingAmt = newDownsampling;
//
//    if (mDownsamplingAmt != mPrevDownsamplingAmt)
//    {
//        mPrevDownsamplingAmt = mDownsamplingAmt;
//        
//        // pre filters
//        preFilter1.reset();
//        preFilter2.reset();
//        preFilter3.reset();
//        preFilter4.reset();
//
//        // post filters
//        postFilter1.reset();
//        postFilter2.reset();
//        postFilter3.reset();
//        postFilter4.reset();
//
//        mFilterCoefficientsArray = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod((mSampleRate / mDownsamplingAmt) * 0.4, mSampleRate, 8);
//
//        // pre filters
//        preFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
//        preFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
//        preFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
//        preFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
//
//        // post filters
//        postFilter1.coefficients = mFilterCoefficientsArray.getObjectPointer(0);
//        postFilter2.coefficients = mFilterCoefficientsArray.getObjectPointer(1);
//        postFilter3.coefficients = mFilterCoefficientsArray.getObjectPointer(2);
//        postFilter4.coefficients = mFilterCoefficientsArray.getObjectPointer(3);
//    }
//}
