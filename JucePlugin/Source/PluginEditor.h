/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginSelectWindow.h"

//==============================================================================
/**
*/
class JucePluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PluginSelectWindow* psw;
    JucePluginAudioProcessorEditor(JucePluginAudioProcessor&);
    ~JucePluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    JucePluginAudioProcessor* myprocessor = nullptr;
    juce::TextEditor* debugShow = nullptr;
    juce::Button* selectScriptBtn = nullptr;
    juce::Button* selectPluginBtn = nullptr;
    juce::Button* toggleDbgBtn = nullptr;
    juce::Button* toggleBypassButton = nullptr;
    juce::String debugOutput;
    juce::DialogWindow* pluginWindow = nullptr;
    void mouseDoubleClick(const juce::MouseEvent& event);
    void mouseDown(const juce::MouseEvent& event);
    void handleCommandMessage(int commandId);
    void initializePluginWindow();
    void MinimisationStateChanged(bool state);
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JucePluginAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucePluginAudioProcessorEditor)
};
