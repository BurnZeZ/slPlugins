/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

using namespace juce::dsp;


#define PARAM_MUTE_L      "muteL"
#define PARAM_LEVEL_L     "levelL"
#define PARAM_MUTE_R      "muteR"
#define PARAM_LEVEL_R     "levelR"

class ChannelMuteAudioProcessorEditor;
//==============================================================================
/**
*/
class ChannelMuteAudioProcessor : public gin::GinProcessor
{
public:
    //==============================================================================
    ChannelMuteAudioProcessor();
    ~ChannelMuteAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    
private:
    Component::SafePointer<ChannelMuteAudioProcessorEditor> editor;

    LinearSmoothedValue<float> lVal;
    LinearSmoothedValue<float> rVal;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelMuteAudioProcessor)
};
