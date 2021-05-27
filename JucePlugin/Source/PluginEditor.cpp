/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginSelectWindow.h"
#include "PluginSelectionListener.h"
#include "RectArranger.h"

//==============================================================================
JucePluginAudioProcessorEditor::JucePluginAudioProcessorEditor (JucePluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    myprocessor = (JucePluginAudioProcessor*)&processor;

    //Configure visual elements
    debugShow = new juce::TextEditor("DBGTEXT");
    debugShow->setFont(juce::Font("JetBrains Mono", 15, 0));
    debugShow->setMultiLine(true, true);
    debugShow->setReadOnly(true);
    debugShow->setText(myprocessor->debugOutput, true);
    debugShow->addMouseListener(this, true);
    addAndMakeVisible(debugShow);

    selectScriptBtn = new juce::TextButton;
    selectScriptBtn->addMouseListener(this,false);
    selectScriptBtn->setButtonText("Select Script");
    addAndMakeVisible(selectScriptBtn);

    selectPluginBtn = new juce::TextButton;
    selectPluginBtn->addMouseListener(this, false);
    selectPluginBtn->setButtonText("Select Plugin");
    addAndMakeVisible(selectPluginBtn);

    toggleDbgBtn = new juce::TextButton;
    toggleDbgBtn->addMouseListener(this, false);
    toggleDbgBtn->setButtonText(myprocessor->debugOutputEnabled ? "NoDBG" : "ShowDBG");
    addAndMakeVisible(toggleDbgBtn);

    toggleBypassButton = new juce::TextButton;
    toggleBypassButton->addMouseListener(this, false);
    toggleBypassButton->setButtonText(myprocessor->bypassed?"Enable":"Bypass");
    addAndMakeVisible(toggleBypassButton);

    autoBypassButton = new juce::TextButton;
    autoBypassButton->addMouseListener(this, false);
    autoBypassButton->setButtonText(myprocessor->autoBypass ? "NoAutoBps" : "AutoBypass");
    addAndMakeVisible(autoBypassButton);

    initializePluginWindow();
}

JucePluginAudioProcessorEditor::~JucePluginAudioProcessorEditor()
{
    if (pluginWindow)
    {
        pluginWindow = nullptr;
        delete pluginWindow;
    }
}

//==============================================================================
void JucePluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    auto children = getChildren();
    for (auto& c : children)
        c->paint(g);
    //g.fillAll(juce::Colour(255,255,255));
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    //AudioProcessorEditor::paint(g);
}

void JucePluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    RectArranger a = getBounds();
    auto b = a.BisectD(30);
    if(debugShow)
        debugShow->setBounds(b.first);

    juce::Array<RectArranger> buttons;
    buttons = b.second.EqualSplitHorizonal(5);
    if (selectPluginBtn)
        selectPluginBtn->setBounds(buttons[0]);
    if(selectScriptBtn)
        selectScriptBtn->setBounds(buttons[1]);
    if (toggleBypassButton)
        toggleBypassButton->setBounds(buttons[2]);
    if (toggleDbgBtn)
        toggleDbgBtn->setBounds(buttons[3]);
    if (autoBypassButton)
        autoBypassButton->setBounds(buttons[4]);
    AudioProcessorEditor::resized();
}

void JucePluginAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    myprocessor->debugOutput = "";
    debugOutput = "";
    postCommandMessage(1);
}

void JucePluginAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    if (event.eventComponent == selectScriptBtn)
    {
        juce::String defaultScript = "C:\\Users\\Fizz\\source\\repos\\JucePlugin\\JucePlugin\\Transformers\\test.lua";
        juce::WildcardFileFilter wildcardFilter("*.lua", juce::String(), "lua script");
        juce::FileBrowserComponent browser(
            juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::openMode,
            juce::File(),
            &wildcardFilter,
            nullptr);
        browser.setRoot(juce::File("C:/ProgramData/MIDI Transformer/scripts/"));
        juce::FileChooserDialogBox dialogBox("Select Transform Script",
            "",
            browser,
            false,
            findColour(juce::TextEditor::ColourIds::backgroundColourId));
        if (dialogBox.show())
        {
            juce::String filename = browser.getSelectedFile(0).getFullPathName();
            myprocessor->initScript(filename);
        }       
    }
    else if (event.eventComponent == selectPluginBtn)
    {
        //juce::WildcardFileFilter wildcardFilter("*.dll;*.vst3", juce::String(), "VST Plugin");
        //juce::FileBrowserComponent browser(
        //    juce::FileBrowserComponent::canSelectFiles,
        //    juce::File(),
        //    &wildcardFilter,
        //    nullptr);

        //juce::FileChooserDialogBox dialogBox("Select Plugin",
        //    "",
        //    browser,
        //    false,
        //    juce::Colours::lightgrey);

        //while (!dialogBox.show());
        //juce::String filename = browser.getSelectedFile(0).getFullPathName();
            //Plugin select / Show hosted editor
        psw = new PluginSelectWindow;
        psw->enterModalState(true, nullptr, false);
        if (psw->runModalLoop())
        {
            myprocessor->pluginLoaded = myprocessor->initPlugin(psw->selected);
            initializePluginWindow();
        }
        delete psw;
        //pluginWindow = new HostedPluginWindow(e,this);
        //pluginWindow->setVisible(true);
        //pluginWindow->addToDesktop();

        //juce::DocumentWindow* tmp = new juce::DocumentWindow("", juce::Colour(127, 127, 127), 1);
        //tmp->setVisible(true);
        //tmp->addToDesktop();
    }
    else if (event.eventComponent == toggleDbgBtn)
    {
        debugOutput = "";
        myprocessor->debugOutput = "";
        postCommandMessage(1);
        myprocessor->debugOutputEnabled = !myprocessor->debugOutputEnabled;
        if (myprocessor->scriptInitialized)
            myprocessor->tf->debugOutputEnabled = !myprocessor->tf->debugOutputEnabled;
        toggleDbgBtn->setButtonText(myprocessor->debugOutputEnabled ? "NoDBG" : "ShowDBG");
    }
    else if (event.eventComponent == toggleBypassButton)
    {
        myprocessor->setBypassed(!myprocessor->bypassed);
        toggleBypassButton->setButtonText(myprocessor->bypassed ? "Enable" : "Bypass");
    }
    else if (event.eventComponent == autoBypassButton)
    {
        myprocessor->setAutoBypass(!myprocessor->autoBypass);
        autoBypassButton->setButtonText(myprocessor->autoBypass ? "NoAutoBps" : "AutoBypass");
    }
}

void JucePluginAudioProcessorEditor::handleCommandMessage(int commandId)
{
    if (commandId == 1)
    {
        if (debugShow)
        {
            debugShow->setText(debugOutput);
            debugShow->moveCaretToEnd();
            debugShow->scrollEditorToPositionCaret(0, debugShow->getHeight() - 30);
        }
    }   
    else if (commandId == 2)
    {
        if (pluginWindow)
        {
            delete pluginWindow;
            pluginWindow = nullptr;
        }
            
        myprocessor->csWaitEditorThread.enter();
        myprocessor->csWaitEditorThread.exit();
    }
    else if (commandId == 3)
    {
        if (pluginWindow)
            pluginWindow->setMinimised(false);
    }
    else if (commandId == 4)
    {
        if (pluginWindow)
            pluginWindow->setMinimised(true);
    }
    else if (commandId == 5)
    {
        initializePluginWindow();
    }
}

void JucePluginAudioProcessorEditor::initializePluginWindow()
{
    if (!myprocessor->pluginLoaded)
        return;
    if (pluginWindow)
    {
        delete pluginWindow;
        pluginWindow = nullptr;
    }
        
    AudioProcessorEditor* e = myprocessor->hostedPlugin->createEditorIfNeeded();
    auto tmp = new juce::DialogWindow("Hosted Plugin", juce::Colour(100, 100, 100), false, false);
    tmp->setContentComponent(e, true, true);
    tmp->setVisible(true);
    tmp->setBounds(0, 0, 100, 100);
    tmp->setTitleBarButtonsRequired(0, false);
    tmp->addToDesktop();

    pluginWindow = tmp;
}

void JucePluginAudioProcessorEditor::MinimisationStateChanged(bool state)
{
    if (pluginWindow)
        pluginWindow->minimisationStateChanged(state);
}

void JucePluginAudioProcessorEditor::ProcessorWait()
{
    myprocessor->csWaitEditorThread.enter();
}