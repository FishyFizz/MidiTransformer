/*
  ==============================================================================

    PluginSelectionListener.h
    Created: 20 May 2021 2:38:30am
    Author:  Fizz

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "PluginSelectWindow.h"
class PluginSelectionListener : public juce::MouseListener
{
public:
    juce::PluginListComponent* AssociatedComponent;
    juce::DialogWindow* Window;
    void mouseDoubleClick(const juce::MouseEvent& me) override
    {
        if (me.eventComponent->getComponentID() != "PLC")
        {
            Window->escapeKeyPressed();
            delete AssociatedComponent;
        }     
    }
};