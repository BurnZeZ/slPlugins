/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
slToneAudioProcessorEditor::slToneAudioProcessorEditor (slToneAudioProcessor& p)
    : slAudioProcessorEditor (&p), processor (p), meter (p.getOutputLevel())
{
    for (slParameter* pp : p.getPluginParameters())
    {
        Knob* k = new Knob (pp);
        
        addAndMakeVisible (k);
        controls.add (k);
    }
    
    addAndMakeVisible (&meter);
    
    setSize (600, 150);
}

slToneAudioProcessorEditor::~slToneAudioProcessorEditor()
{
}

//==============================================================================
void slToneAudioProcessorEditor::paint(Graphics& g)
{
    g.fillAll (Colours::white);
}

void slToneAudioProcessorEditor::resized()
{
    Rectangle<int> r = getControlsArea();
    
    meter.setBounds (r.removeFromRight (15));
    
    int w = r.getWidth() / controls.size();
    
    for (auto c : controls)
        c->setBounds (r.removeFromLeft (w));
}
