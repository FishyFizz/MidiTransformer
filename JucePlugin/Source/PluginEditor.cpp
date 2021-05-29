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

    selectScriptBtn = AddButtonWithText<juce::TextButton>("open script");
    
    selectPluginBtn = AddButtonWithText<juce::TextButton>("open plugin");

    toggleDbgBtn = AddButtonWithText<juce::ToggleButton>("show dbgout");
    toggleDbgBtn->setClickingTogglesState(false);

    toggleBypassButton = AddButtonWithText<juce::ToggleButton>("bypass");
    toggleBypassButton->setClickingTogglesState(false);

    autoBypassButton = AddButtonWithText<juce::ToggleButton>("autobypass");
    autoBypassButton->setClickingTogglesState(false);

    showPluginWindowToggleButton = AddButtonWithText<juce::ToggleButton>("show plugin");
    showPluginWindowToggleButton->setClickingTogglesState(false);

    settingsButton = new juce::ImageButton("");
    settingsButton->addMouseListener(this, false);
    juce::MemoryBlock imagedata;
    juce::File("C:/ProgramData/MIDI Transformer/assets/settingsicon.png").loadFileAsData(imagedata);
    juce::Image image = juce::PNGImageFormat::loadFrom(imagedata.getData(), imagedata.getSize());
    settingsButton->setImages(
        false,
        true,
        true,
        image,
        1,
        juce::Colour(juce::uint8(0), 0, 0, juce::uint8(0)),
        juce::Image(),
        1,
        juce::Colour(juce::uint8(255), 255, 255, juce::uint8(50)),
        juce::Image(),
        1,
        juce::Colour(juce::uint8(255), 255, 255, juce::uint8(50)));
    addAndMakeVisible(settingsButton);

    initializePluginWindow();
    RefreshToggleButtonStates();
}

JucePluginAudioProcessorEditor::~JucePluginAudioProcessorEditor()
{
    SafeDelete(pluginWindow);
    deleteAllChildren();
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

    auto c = b.second.BisectR(30);
    if (settingsButton)
        settingsButton->setBounds(c.second);

    juce::Array<RectArranger> buttons;
    buttons = c.first.EqualSplitHorizonal(6);
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
    if (showPluginWindowToggleButton)
        showPluginWindowToggleButton->setBounds(buttons[5]);
    AudioProcessorEditor::resized();
}

void JucePluginAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (event.eventComponent == debugShow)
    {
        myprocessor->debugOutput = "";
        debugOutput = "";
        postCommandMessage(1);
    }
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
        psw = new PluginSelectWindow;
        psw->enterModalState(true, nullptr, false);
        if (psw->runModalLoop())
        {
            myprocessor->pluginLoaded = myprocessor->initPlugin(psw->selected);
            initializePluginWindow();
        }
        delete psw;
    }
    else if (event.eventComponent == toggleDbgBtn)
    {
        debugOutput = "";
        myprocessor->debugOutput = "";
        postCommandMessage(1);
        myprocessor->setDbgOutEnable(!myprocessor->debugOutputEnabled);
    }
    else if (event.eventComponent == toggleBypassButton)
    {
        myprocessor->setUserBypassState(!myprocessor->bypassed);
    }
    else if (event.eventComponent == autoBypassButton)
    {
        myprocessor->setAutoBypass(!myprocessor->autoBypass);
    }
    else if (event.eventComponent == showPluginWindowToggleButton)
    {
        if (pluginWindow)
            postCommandMessage(2);
        else
            initializePluginWindow();
    }
    else if (event.eventComponent == settingsButton)
    {
        EditSearchPathWindow* dlg = new EditSearchPathWindow;
        dlg->setBounds(dlg->getBounds().withCentre(getBounds().getCentre()+getScreenPosition()));
        dlg->addToDesktop();
        dlg->runModalLoop();
    }
}

void JucePluginAudioProcessorEditor::handleCommandMessage(int commandId)
{
    if (commandId == 1) //Refresh dbgShow text
    {
        if (debugShow)
        {
            if (debugOutput.length() > 3000)
            {
                myprocessor->debugOutput = debugOutput = debugOutput.substring(1000);
            }
            debugShow->setText(debugOutput);
            debugShow->moveCaretToEnd();
            debugShow->scrollEditorToPositionCaret(0, debugShow->getHeight() - 30);
        }
    }   
    else if (commandId == 2) //close plugin window
    {
        SafeDelete(pluginWindow);
        //myprocessor->waitCloseHostedEditor.signal();
        RefreshToggleButtonStates();
    }   
    else if (commandId == 5) //initialize plugin window
    {
        initializePluginWindow();
    }
    else if (commandId == 6) //refresh toggle button states
    {
        RefreshToggleButtonStates();
    }
}

void JucePluginAudioProcessorEditor::initializePluginWindow()
{
    if (!myprocessor->pluginLoaded)
        return;
    SafeDelete(pluginWindow);
    AudioProcessorEditor* e = myprocessor->hostedPlugin->createEditorIfNeeded();
    auto tmp = new HostedWindow(this);
    tmp->setContentComponent(e, true, true);
    tmp->setVisible(true);
    tmp->setBounds(0, 0, 100, 100);
    tmp->addToDesktop();

    pluginWindow = tmp;
    RefreshToggleButtonStates();
}

void JucePluginAudioProcessorEditor::RefreshToggleButtonStates()
{
    autoBypassButton->setToggleState(myprocessor->autoBypass, juce::NotificationType::dontSendNotification);
    toggleBypassButton->setToggleState(myprocessor->bypassed, juce::NotificationType::dontSendNotification);
    toggleDbgBtn->setToggleState(myprocessor->debugOutputEnabled, juce::NotificationType::dontSendNotification);
    showPluginWindowToggleButton->setToggleState(pluginWindow, juce::NotificationType::dontSendNotification);
}

template <class T>
T* JucePluginAudioProcessorEditor::AddButtonWithText(const char* text)
{
    T* tmp = new T;
    tmp->addMouseListener(this, false);
    tmp->setButtonText(text);
    addAndMakeVisible(tmp);
    return tmp;
}