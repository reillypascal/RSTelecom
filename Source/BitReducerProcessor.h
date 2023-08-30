#pragma once

#include <JuceHeader.h>
#include "Utilities.h"

// Mu-law encoding, copyright 2014 Emilie Gillet.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//

class MuLawProcessor : public CodecProcessorBase
{
public:
    MuLawProcessor();
    
    ~MuLawProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
    
private:
    unsigned char Lin2MuLaw(int16_t pcm_val);
    
    short MuLaw2Lin(uint8_t u_val);
    
    float mOutScale { 1.0f/32767.0f };
    
    int mSampleRate { 44100 };
    int mNumChannels { 2 };
    std::vector<int> mDownsamplingCounter { 0, 0 };
    CodecProcessorParameters parameters;
    
    int mResamplingFilterOrder = 8;
    
    using IIR = juce::dsp::IIR::Filter<float>;
    std::vector<std::vector<IIR>> preFilters;
    std::vector<std::vector<IIR>> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
};


//=======================================================================

class ALawProcessor : public CodecProcessorBase
{
public:
    ALawProcessor();
    
    ~ALawProcessor() override;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    void reset() override;
    
    CodecProcessorParameters& getParameters() override;
    
    void setParameters(const CodecProcessorParameters& params) override;
    
private:
//    unsigned char Lin2ALaw(int16_t pcm_val);
    
//    short ALaw2Lin(uint8_t u_val);
    
    float mOutScale { 1.0f/32767.0f };
    
    int mSampleRate { 44100 };
    int mNumChannels { 2 };
    std::vector<int> mDownsamplingCounter { 0, 0 };
    CodecProcessorParameters parameters;
    
    int mResamplingFilterOrder = 8;
    
    using IIR = juce::dsp::IIR::Filter<float>;
    std::vector<std::vector<IIR>> preFilters;
    std::vector<std::vector<IIR>> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
};
