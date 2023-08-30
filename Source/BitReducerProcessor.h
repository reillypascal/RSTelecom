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
    int mResamplingFilterOrder { 8 };
    std::vector<int> mDownsamplingCounter { 0, 0 };
    std::vector<float> mDownsamplingInput { 0.0f, 0.0f };
    std::vector<float> mDownsamplingGainComp { 1.0f, 1.0f, 1.22f, 1.675f, 2.25f, 2.675f, 3.25f, 3.675f, 4.0f };
    
    CodecProcessorParameters parameters;
    
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
    unsigned char Lin2ALaw(int16_t pcm_val);
    
    short ALaw2Lin(uint8_t u_val);
    
    const int cBias = 0x84;
    const int cClip = 32635;
    constexpr static char ALawCompressTable[128]
    {
        1,1,2,2,3,3,3,3,
        4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,
        5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7
    };
    
    constexpr static short ALawDecompressTable[256] =
    {
         -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
         -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
         -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
         -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
         -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
         -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
         -11008,-10496,-12032,-11520,-8960, -8448, -9984, -9472,
         -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
         -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
         -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
         -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40,
         -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
         -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
         -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
         -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
         -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
          5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
          7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
          2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
          3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
          22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
          30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
          11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472,
          15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
          344,   328,   376,   360,   280,   264,   312,   296,
          472,   456,   504,   488,   408,   392,   440,   424,
          88,    72,   120,   104,    24,     8,    56,    40,
          216,   200,   248,   232,   152,   136,   184,   168,
          1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
          1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
          688,   656,   752,   720,   560,   528,   624,   592,
          944,   912,  1008,   976,   816,   784,   880,   848
    };
    
    float mOutScale { 1.0f/32767.0f };
    
    int mSampleRate { 44100 };
    int mNumChannels { 2 };
    int mResamplingFilterOrder { 8 };
    std::vector<int> mDownsamplingCounter { 0, 0 };
    std::vector<float> mDownsamplingInput { 0.0f, 0.0f };
    std::vector<float> mDownsamplingGainComp { 1.0f, 1.0f, 1.22f, 1.675f, 2.25f, 2.675f, 3.25f, 3.675f, 4.0f };
    
    CodecProcessorParameters parameters;
    
    using IIR = juce::dsp::IIR::Filter<float>;
    std::vector<std::vector<IIR>> preFilters;
    std::vector<std::vector<IIR>> postFilters;
    
    juce::ReferenceCountedArray<juce::dsp::IIR::Coefficients<float>> mFilterCoefficientsArray;
};
