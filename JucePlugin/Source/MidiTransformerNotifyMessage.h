/*
  ==============================================================================

    MidiTransformerNotifyMessage.h
    Created: 23 May 2021 4:33:17am
    Author:  Fizz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MidiTransformerNotifyMessage : juce::Message
{
public:
    juce::MidiBuffer midiMessages;
    MidiTransformerNotifyMessage(const juce::MidiBuffer& buf) { midiMessages = buf; }
};