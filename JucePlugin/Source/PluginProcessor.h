/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiTransformer.h"

//==============================================================================
/**
*/
class JucePluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    JucePluginAudioProcessor();
    ~JucePluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //CUSTOMIZED====================================================================
    std::unique_ptr<juce::AudioPluginInstance> hostedPlugin;
    bool initPlugin(juce::PluginDescription& const desc, const void* PluginChunk = nullptr, int PluginChunkSize = 0);
    void unloadPlugin();
    MidiTransformer* tf;
    void initScript(juce::String scriptfile);
    juce::String debugOutput;
    void ReportLatency();
    
    void setScriptInitialized(bool b);
    void setAutoBypass(bool b);
    void setBypassed(bool b);
    void setDbgOutEnable(bool b);

    void refreshEditorToggleButton();

    juce::WaitableEvent waitCloseHostedEditor;
    void tellEditorClosePluginEditorWait();

    void luaFail();
    //==============================================================================
    bool scriptInitialized = false;
    bool pluginLoaded = false;
    bool autoBypass = true;
    bool debugOutputEnabled = true;
    bool bypassed = false;
    juce::PluginDescription desc;
    juce::String scriptFileName;

    //Memory Block Writing Helpers==================================================

#ifdef DEBUG
    long long debugCounter=0;
#endif
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucePluginAudioProcessor)
};
