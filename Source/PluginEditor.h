// JUCE Editor

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI.h"

//==============================================================================
/**
*/
class RSTelecomAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RSTelecomAudioProcessorEditor (RSTelecomAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~RSTelecomAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    juce::Label downsamplingLabel;
    juce::Label bitrateLabel;
    juce::Label saturationLabel;
    
    juce::Label errorClockLabel;
    juce::Label errorProbLabel;
    
    juce::Label slot1Label;
    juce::Label slot2Label;
    
    juce::Slider downsamplingSlider;
    juce::Slider bitrateSlider;
    juce::Slider saturationSlider;
    
    juce::Slider errorClockSlider;
    juce::Slider errorProbSlider;
    
    juce::ComboBox slot1Menu;
    juce::ComboBox slot2Menu;
    
    std::unique_ptr<SliderAttachment> downsamplingAttachment;
    std::unique_ptr<SliderAttachment> bitrateAttachment;
    std::unique_ptr<SliderAttachment> saturationAttachment;
    
    std::unique_ptr<SliderAttachment> errorClockAttachment;
    std::unique_ptr<SliderAttachment> errorProbAttachment;
    
    std::unique_ptr<ComboBoxAttachment> slot1MenuAttachment;
    std::unique_ptr<ComboBoxAttachment> slot2MenuAttachment;
    
    enum class CodecMode
    {
        none = 1,
        gsm610,
        mulaw,
        alaw,
        vox
    };
    
    enum class DownsamplingModes
    {
        none = 1,
        x2,
        x3,
        x4,
        x5,
        x6,
        x7,
        x8
    };
    
    const int textBoxWidth = 70;
    const int textBoxHeight = 25;
    GrayBlueLookAndFeel grayBlueLookAndFeel;
    
    RSTelecomAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RSTelecomAudioProcessorEditor)
};
