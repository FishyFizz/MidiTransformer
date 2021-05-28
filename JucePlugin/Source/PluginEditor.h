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

    class HostedWindow : public juce::DialogWindow
    {
    public:
        JucePluginAudioProcessorEditor* parent;
        explicit HostedWindow(JucePluginAudioProcessorEditor* parent):DialogWindow("Hosted Plugin", juce::Colour(100, 100, 100), true, false)
        {
            this->parent = parent;
            setTitleBarButtonsRequired(juce::DocumentWindow::TitleBarButtons::closeButton,false);
        }
        void closeButtonPressed() override
        {
            //DialogWindow::closeButtonPressed();
            parent->postCommandMessage(2);
        }
        ~HostedWindow()
        {
            SafeDelete(getContentComponent());
        }
    };

    class EditSearchPathWindow : public juce::DialogWindow
    {
    public:
        juce::TextEditor* edt;
        EditSearchPathWindow():DialogWindow("Edit plugin paths",juce::Colour(0,0,0),true,false)
        {
            juce::StringArray arr;
            juce::File("C:/ProgramData/MIDI Transformer/searchpaths.txt").readLines(arr);

            edt = new juce::TextEditor;
            edt->setSize(600, 400);
            edt->setMultiLine(true);
            edt->setReturnKeyStartsNewLine(true);
            edt->setText(arr.joinIntoString("\n"));
            setContentComponent(edt,true,true);
        }
        void closeButtonPressed() override
        {
            juce::File("C:/ProgramData/MIDI Transformer/searchpaths.txt").replaceWithText(edt->getText());
            delete this;
        }
        ~EditSearchPathWindow()
        {
            delete edt;
        }
    };

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
    juce::Button* autoBypassButton = nullptr;
    juce::Button* showPluginWindowToggleButton = nullptr;
    juce::ImageButton* settingsButton = nullptr;
    juce::String debugOutput;
    HostedWindow* pluginWindow = nullptr;
    void mouseDoubleClick(const juce::MouseEvent& event);
    void mouseDown(const juce::MouseEvent& event);
    void handleCommandMessage(int commandId);
    void initializePluginWindow();

    template <class T>
    static void SafeDelete(T*& ptr) { if (ptr) delete ptr; ptr = nullptr; }
    template <class T>
    static void SafeDelete(T* &&ptr) { if (ptr) delete ptr;}

    template <class T>
    T* AddButtonWithText(const char* text);

    void RefreshToggleButtonStates();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JucePluginAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucePluginAudioProcessorEditor)
};
