/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RSTelecomAudioProcessorEditor::RSTelecomAudioProcessorEditor (RSTelecomAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), valueTreeState(vts), audioProcessor (p)
{
    // labels row 1
    downsamplingLabel.setText("Downsampling", juce::dontSendNotification);
    downsamplingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(downsamplingLabel);
    
    bitrateLabel.setText("Bitrate", juce::dontSendNotification);
    bitrateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bitrateLabel);
    
    saturationLabel.setText("Saturation", juce::dontSendNotification);
    saturationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationLabel);
    
    // labels row 2
    errorClockLabel.setText("Error Clock", juce::dontSendNotification);
    errorClockLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(errorClockLabel);
    
    errorProbLabel.setText("Error Prob", juce::dontSendNotification);
    errorProbLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(errorProbLabel);
    
    // menu labels
    slot1Label.setText("Processor Slot 1", juce::dontSendNotification);
    slot1Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(slot1Label);
    
    slot2Label.setText("Processor Slot 2", juce::dontSendNotification);
    slot2Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(slot2Label);
    
    // sliders row 1
    downsamplingSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    downsamplingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(downsamplingSlider);
    downsamplingAttachment.reset(new SliderAttachment(valueTreeState, "downsampling", downsamplingSlider));
    
    bitrateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    bitrateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(bitrateSlider);
    bitrateAttachment.reset(new SliderAttachment(valueTreeState, "bitrate", bitrateSlider));
    
    saturationSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(saturationSlider);
    saturationAttachment.reset(new SliderAttachment(valueTreeState, "saturation", saturationSlider));
    
    // sliders row 2
    errorClockSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    errorClockSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(errorClockSlider);
    errorClockAttachment.reset(new SliderAttachment(valueTreeState, "errorClock", errorClockSlider));
    
    errorProbSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    errorProbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxWidth, textBoxHeight);
    addAndMakeVisible(errorProbSlider);
    errorProbAttachment.reset(new SliderAttachment(valueTreeState, "errorProb", errorProbSlider));
    
    // menus
    addAndMakeVisible(slot1Menu);
    using enum CodecMode;
    slot1Menu.addItem("None", static_cast<int>(none));
    slot1Menu.addItem("GSM 06.10", static_cast<int>(gsm610));
//    slot1Menu.addItem("iLBC", static_cast<int>(ilbc));
    slot1Menu.addItem("Mu-Law", static_cast<int>(mulaw));
    slot1Menu.addItem("A-Law", static_cast<int>(alaw));
    slot1Menu.setSelectedId(static_cast<int>(none));
    slot1Menu.setTextWhenNothingSelected("Codec:");
    slot1Menu.setJustificationType(juce::Justification::centred);
    slot1MenuAttachment.reset(new ComboBoxAttachment(valueTreeState, "slot1", slot1Menu));
    
    addAndMakeVisible(slot2Menu);
    slot2Menu.addItem("None", static_cast<int>(none));
    slot2Menu.addItem("GSM 06.10", static_cast<int>(gsm610));
//    slot2Menu.addItem("iLBC", static_cast<int>(ilbc));
    slot2Menu.addItem("Mu-Law", static_cast<int>(mulaw));
    slot2Menu.addItem("A-Law", static_cast<int>(alaw));
    slot2Menu.setSelectedId(static_cast<int>(none));
    slot2Menu.setTextWhenNothingSelected("Codec:");
    slot2Menu.setJustificationType(juce::Justification::centred);
    slot2MenuAttachment.reset(new ComboBoxAttachment(valueTreeState, "slot2", slot2Menu));
    
    getLookAndFeel().setDefaultLookAndFeel(&grayBlueLookAndFeel);
    
    setSize (700, 550);
}

RSTelecomAudioProcessorEditor::~RSTelecomAudioProcessorEditor()
{
}

//==============================================================================
void RSTelecomAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour::fromRGB(32, 32, 32));

    g.setColour (juce::Colours::aliceblue);
    g.setFont (juce::Font(32.0f, juce::Font::plain));
    g.drawFittedText ("RS Telecom", 25, 10, 350, 45, juce::Justification::left, 1);
    
    g.setFont(juce::Font(16.0f, juce::Font::plain));
    g.drawFittedText("Version 0.1.0\n reillyspitzfaden.com", getWidth() - 375, 15, 350, 45, juce::Justification::right, 2);
    
    g.setColour(juce::Colour::fromRGB(68, 81, 96));
    g.fillRoundedRectangle(25, 65, getWidth() - 50, 230, 25);
    g.fillRoundedRectangle(25, 320, (getWidth() / 2) - 25, 200, 25);
    g.fillRoundedRectangle(getWidth() / 2 + 25, 320, (getWidth() / 2) - 50, 200, 25);
}

void RSTelecomAudioProcessorEditor::resized()
{
    const int xBorder = 30;
    const int yBorderTop = 85;
    const int yBorderBottom = 35;
    const int rowSpacer = 105;
    
    const int menuWidth = 150;
    const int menuHeight = 20;
    const int sliderWidth1 = (getWidth() - (2 * xBorder)) / 3;
    const int sliderWidth2 = (getWidth() - (2 * xBorder)) / 4;
    const int sliderHeight1 = (getHeight() - yBorderTop - yBorderBottom - rowSpacer - menuHeight) / 2;
    const int sliderHeight2 = sliderHeight1 * 0.8;
    const int textLabelWidth = 150;
    const int textLabelHeight = 20;
    const int textLabelSpacer = 7;
    
    // row 1 sliders
    downsamplingSlider.setBounds(xBorder,
                                 yBorderTop,
                                 sliderWidth1,
                                 sliderHeight1);
    bitrateSlider.setBounds(xBorder + sliderWidth1,
                            yBorderTop,
                            sliderWidth1,
                            sliderHeight1);
     saturationSlider.setBounds(xBorder + (2 * sliderWidth1),
                                yBorderTop,
                                sliderWidth1,
                                sliderHeight1);
    
    // row 2 sliders
    errorClockSlider.setBounds(xBorder,
                               yBorderTop + sliderHeight1 + rowSpacer,
                               sliderWidth2,
                               sliderHeight2);
    errorProbSlider.setBounds(xBorder + sliderWidth2,
                              yBorderTop + sliderHeight1 + rowSpacer,
                              sliderWidth2,
                              sliderHeight2);
    
    // row 1 labels
    downsamplingLabel.setBounds(xBorder + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                                yBorderTop + sliderHeight1 + textLabelSpacer,
                                textLabelWidth,
                                textLabelHeight);
    bitrateLabel.setBounds(xBorder + sliderWidth1 + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                           yBorderTop + sliderHeight1 + textLabelSpacer,
                           textLabelWidth,
                           textLabelHeight);
    saturationLabel.setBounds(xBorder + (sliderWidth1 * 2) + ((sliderWidth1 / 2) - (textLabelWidth / 2)),
                              yBorderTop + sliderHeight1 + textLabelSpacer,
                              textLabelWidth,
                              textLabelHeight);
    
    // row 2 labels
    errorClockLabel.setBounds(xBorder + ((sliderWidth2 / 2) - (textLabelWidth / 2)),
                         yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                         textLabelWidth,
                         textLabelHeight);
    errorProbLabel.setBounds(xBorder + sliderWidth2 + ((sliderWidth2 / 2) - (textLabelWidth / 2)),
                                yBorderTop + sliderHeight1 + sliderHeight2 + rowSpacer + textLabelSpacer,
                                textLabelWidth,
                                textLabelHeight);
    
    // menus
    slot1Menu.setBounds((getWidth() * 0.75) - (menuWidth / 2),
                        yBorderTop + sliderHeight1 + rowSpacer + 15,
                        menuWidth,
                        menuHeight);
    slot2Menu.setBounds((getWidth() * 0.75) - (menuWidth / 2),
                        yBorderTop + sliderHeight1 + rowSpacer + 100,
                        menuWidth,
                        menuHeight);
    
    // menu labels
    slot1Label.setBounds((getWidth() * 0.75) - (textLabelWidth / 2),
                         yBorderTop + sliderHeight1 + rowSpacer + 45,
                         textLabelWidth,
                         textLabelHeight);
    slot2Label.setBounds((getWidth() * 0.75) - (textLabelWidth / 2),
                         yBorderTop + sliderHeight1 + rowSpacer + 130,
                         textLabelWidth,
                         textLabelHeight);
    
}
