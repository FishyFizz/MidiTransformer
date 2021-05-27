/*
  ==============================================================================

    PluginSelectWindow.cpp
    Created: 20 May 2021 12:24:12am
    Author:  Fizz

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginSelectWindow.h"


//==============================================================================
PluginSelectWindow::PluginSelectWindow() : DialogWindow("PluginSelect",juce::Colour( 127,127,127),false,true)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setResizable(true, true);
    setSize(800, 800);

    //Load plugin list and blacklist--------------------------------------------------------------------
    if (juce::File("C:/ProgramData/MIDI Transformer/pluginlist.xml").exists())
    {
        auto xml = juce::XmlDocument::parse(juce::File("C:/ProgramData/MIDI Transformer/pluginlist.xml"));
        knownList.recreateFromXml(*xml);
    }

    if (juce::File("C:/ProgramData/MIDI Transformer/blacklist.txt").exists())
    {
        blacklistFile = juce::File("C:/ProgramData/MIDI Transformer/blacklist.txt");
        juce::StringArray blacklistedPlugins;
        blacklistFile.readLines(blacklistedPlugins);
        for (juce::String& s : blacklistedPlugins)
            knownList.addToBlacklist(s);
    }
    else
        juce::File("C:/ProgramData/MIDI Transformer/blacklist.txt").create();

    //Dialog layout------------------------------------------------------------------------------------
    pluginListComponent = new juce::PluginListComponent(formatManager, knownList, blacklistFile, NULL);
    pluginListComponent->setBounds(juce::Rectangle<int>(0, getTitleBarHeight(), getWidth(), getHeight() - getTitleBarHeight()));
    pluginListComponent->setComponentID("PLC");
    pluginListComponent->getTableListBox().setComponentID("PTLB");
    pluginListComponent->getTableListBox().addMouseListener(new SelectionListener(this), true);
    setComponentID("PSW");
    setContentOwned(pluginListComponent, false);
    //Actual scan process-------------------------------------------------------------------------------

    formatManager.addFormat(new juce::VST3PluginFormat);

    juce::StringArray searchDirsArray;
    juce::File pathsFile("C:/ProgramData/MIDI Transformer/searchpaths.txt");
    pathsFile.readLines(searchDirsArray);

    for (auto& dir : searchDirsArray)
        searchDir.addIfNotAlreadyThere(juce::File(dir));

    juce::PluginDirectoryScanner scanner(knownList, Format, searchDir, true, blacklistFile);
    juce::String tmpName;
    while (scanner.scanNextFile(true, tmpName));
}

PluginSelectWindow::~PluginSelectWindow()
{

}

void PluginSelectWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    DialogWindow::paint(g);
}

void PluginSelectWindow::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    DialogWindow::resized();
}

void PluginSelectWindow::closeButtonPressed()
{
    //Save scanned results-----------------------------------------------------------------------------
    if (knownList.getNumTypes())
    {
        auto pluginsXml = knownList.createXml();
        pluginsXml->writeTo(juce::File("C:/ProgramData/MIDI Transformer/pluginlist.xml"));
    }
    exitModalState(hasSelection);
}
