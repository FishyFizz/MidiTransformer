/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiTransformer.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;
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
    juce::StringArray debugMessages;
    void ReportLatency();
    
    void setScriptInitialized(bool b);
    void setAutoBypass(bool b, bool notifyOtherInstances = true);
    void setBypassed(bool b, bool notifyOtherInstances = true);
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

    //IPC Implementation============================================================
    class MidiTransformerIPCSync : public juce::Thread
    {
    public:
        JucePluginAudioProcessor* parent;
        
        MidiTransformerIPCSync(JucePluginAudioProcessor* _parent) :Thread("MidiTransformerIPCSyncThread") { parent = _parent; }

        struct MidiTransformerSyncedProperties
        {
            bool autoBypass;
            bool bypassed;
            int instancesCount;
            ThreadID threadBroadcaster; // to identify if the change comes from the thread itself.

            // when received a boradcast from other instance, increase this.
            // when all instances have reported received the broadcast, the broadcaster might do some spesified job
            int broadcastResponseCounter; 
                
            bool operator!=(const MidiTransformerSyncedProperties& b)
            {
                return autoBypass != b.autoBypass || instancesCount != b.instancesCount || bypassed != b.bypassed;
            }
        };

        MidiTransformerSyncedProperties properties;
        MidiTransformerSyncedProperties* mappedProperties;
        shared_memory_object memobj;
        mapped_region memmap;
        ThreadID tid;
        bool suspend = false;
        void overwrite(MidiTransformerSyncedProperties& const newproperties)
        {
            //in case if it's called from another thread, then suspend the sync loop
            suspend = true;
            properties = newproperties;
            properties.threadBroadcaster = tid;
            *mappedProperties = properties;
            suspend = false;
        }
        void run() override
        {
            tid = getThreadId();
            memobj = shared_memory_object(open_or_create, "MidiTransformerIpcSync", read_write);
            offset_t memsize;
            memobj.get_size(memsize);
            if (memsize == 0) // newly opened
            {
                // set size of new shared block
                memobj.truncate(sizeof(MidiTransformerSyncedProperties));
                memmap = mapped_region(memobj, read_write);
                mappedProperties = (MidiTransformerSyncedProperties *) memmap.get_address();

                // write data
                properties = { parent->autoBypass,parent->bypassed,1,tid,1};
                *mappedProperties = properties;
            }
            else
            {
                memmap = mapped_region(memobj, read_write);
                mappedProperties = (MidiTransformerSyncedProperties*)memmap.get_address();
                mappedProperties->instancesCount++;
            }

            // change listener loop
            while (!threadShouldExit())
            {
                if (suspend) continue;
                if (*mappedProperties != properties)
                {
                    if (mappedProperties->threadBroadcaster != tid)
                        mappedProperties->broadcastResponseCounter++;
                    properties = *mappedProperties;
                    parent->SyncPropertiesUpdated(properties);
                }
            }

            //Check if instance count is 0, and close shared memory object if needed
            if (mappedProperties->instancesCount == 0)
            {
                shared_memory_object::remove("MidiTransformerIpcSync");
            }
        }
    } *SyncObject = nullptr;
    void SyncPropertiesUpdated(const MidiTransformerIPCSync::MidiTransformerSyncedProperties& properties);

#ifdef DEBUG
    long long debugCounter=0;
#endif
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucePluginAudioProcessor)
};
