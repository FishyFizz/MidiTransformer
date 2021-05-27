/*
  ==============================================================================

    PluginSelectWindow.h
    Created: 20 May 2021 12:24:12am
    Author:  Fizz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class PluginSelectWindow  : public juce::DialogWindow
{
public:

    class SelectionListener : public juce::MouseListener
    {
    public:
        SelectionListener(PluginSelectWindow* p) { parent = p; }
        PluginSelectWindow* parent;
        void mouseDoubleClick(const juce::MouseEvent& event) override
        {
            juce::TableListBox& tlb = parent->pluginListComponent->getTableListBox();
            const auto& cid = event.eventComponent->getComponentID();
            if (cid != "PTLB")
            {
                juce::PluginDescription plugin;
                parent->selected = juce::PluginDescription(*(parent->knownList.getType(tlb.getSelectedRow())));
                parent->hasSelection = true;
                parent->closeButtonPressed();
            }
        }
    };

    PluginSelectWindow();
    ~PluginSelectWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void closeButtonPressed() override;
    
    juce::KnownPluginList knownList;
    juce::File blacklistFile;
    juce::FileSearchPath searchDir;
    juce::AudioPluginFormatManager formatManager;
    juce::PluginListComponent* pluginListComponent;
    juce::VSTPluginFormat Format;
    juce::PluginDescription selected;
    bool hasSelection=false;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginSelectWindow)
};
