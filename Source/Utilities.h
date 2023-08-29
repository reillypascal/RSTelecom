#pragma once

#include <JuceHeader.h>

inline float randomFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline float scale(float input, float inLow, float inHi, float outLow, float outHi)
{
    float scaleFactor = (outHi - outLow)/(inHi - inLow);
    float offset = outLow - inLow;
    return (input * scaleFactor) + offset;
}

inline float wrap(float a, float b)
{
    float mod = fmodf(a, b);
    return (a >= 0 ? 0 : b) + (mod > __FLT_EPSILON__ || !isnan(mod) ? mod : 0);
}

struct CodecProcessorParameters
{
    CodecProcessorParameters() {}
    
    CodecProcessorParameters& operator=(const CodecProcessorParameters& params)
    {
        if (this != &params)
        {
            downsampling = params.downsampling;
            bitrate = params.bitrate;
        }
        return *this;
    }
    
    int downsampling = 1;
    int bitrate = 1;
    // needs glitch-related params
};

class CodecProcessorBase
{
public:
    CodecProcessorBase() {}
    
    virtual ~CodecProcessorBase() {}
    
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;
    
    virtual void reset() = 0;
    
    virtual CodecProcessorParameters& getParameters() = 0;
    
    virtual void setParameters(const CodecProcessorParameters& params) = 0;
};

class LockGuardedPosInfo
{
public:
    void set(const juce::AudioPlayHead::PositionInfo& newInfo)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        info = newInfo;
    }
    
    juce::AudioPlayHead::PositionInfo get() noexcept
    {
        const std::lock_guard<std::mutex> lock(mutex);
        return info;
    }
private:
    std::mutex mutex;
    juce::AudioPlayHead::PositionInfo info;
};
