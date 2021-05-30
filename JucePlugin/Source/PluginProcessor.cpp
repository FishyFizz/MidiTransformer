/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

//==============================================================================
JucePluginAudioProcessor::JucePluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    SyncObject = new MidiTransformerIPCSync(this);
    SyncObject->startThread();
    bypassTimerThread = new TimedSetAutoBypassState(this, 1000, true);
}



//==============================================================================
const juce::String JucePluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JucePluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JucePluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JucePluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JucePluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JucePluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JucePluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JucePluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String JucePluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void JucePluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void JucePluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void JucePluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JucePluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JucePluginAudioProcessor();
}


//==============================================================================
bool JucePluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}


//==============================================================================
void JucePluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream;

    //basic info
    stream.writeBool(autoBypass);
    stream.writeBool(debugOutputEnabled);
    stream.writeBool(bypassed);

    //script info
    stream.writeBool(scriptInitialized);
    if (scriptInitialized)
    {
        stream.writeString(scriptFileName);
    }

    //plugin info
    stream.writeBool(pluginLoaded && hostedPlugin);
    if (pluginLoaded && hostedPlugin)
    {
        juce::MemoryBlock d;
        hostedPlugin->getStateInformation(d);

        stream.writeString(desc.createXml()->toString());
        stream.writeString(desc.createIdentifierString());
        stream.writeInt64(d.getSize());
        stream.write(d.getData(), d.getSize());

        //hostedPlugin->setStateInformation(d.getData(), d.getSize());
    } 

    destData.replaceWith(stream.getData(), stream.getDataSize());
}

void JucePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data,sizeInBytes,true);

    //basic info
    setAutoBypass(stream.readBool());
    debugOutputEnabled = stream.readBool();
    setBypassed(stream.readBool());

    //script info
    bool loadScriptInfo = stream.readBool();
    if (loadScriptInfo)
    {
        scriptFileName = stream.readString();
        initScript(scriptFileName);
    }

    //plugin info
    bool loadPluginInfo = stream.readBool();
    if (loadPluginInfo)
    {
        juce::String descXml, descIdentifier;
        juce::int64 pluginChunkSize;

        descXml = stream.readString();
        descIdentifier = stream.readString();
        pluginChunkSize = stream.readInt64();

        void* pluginChunk = calloc(1, pluginChunkSize);
        stream.read(pluginChunk, pluginChunkSize);

        desc.loadFromXml(*(juce::XmlDocument(descXml).getDocumentElement(false)));
        initPlugin(desc, pluginChunk, pluginChunkSize);
        free(pluginChunk);
    }
}

//CUSTOMIZED====================================================================

bool JucePluginAudioProcessor::initPlugin(juce::PluginDescription& const desc, const void* PluginChunk, int PluginChunkSize)
{
    if (pluginLoaded)
        unloadPlugin();
        
    juce::VST3PluginFormat f3;
    hostedPlugin = f3.createInstanceFromDescription(desc, getSampleRate(), 512);
    if (!hostedPlugin)
    {
        juce::VSTPluginFormat f;
        hostedPlugin = f.createInstanceFromDescription(desc, getSampleRate(), 512);
    }

    if (hostedPlugin)
    {
        if (PluginChunk != nullptr)
            hostedPlugin->setStateInformation(PluginChunk, PluginChunkSize);
        this->desc = desc;
        pluginLoaded = true;
    }
    return true;
}

void JucePluginAudioProcessor::unloadPlugin()
{
    if (!pluginLoaded)
        return;
    pluginLoaded = false;
    tellEditorClosePluginEditorWait();
}

void JucePluginAudioProcessor::initScript(juce::String scriptfile = juce::String())
{
    setScriptInitialized(false);
    tf = new MidiTransformer;
    tf->LatencySmps = 44100;
    tf->debugOutputEnabled = debugOutputEnabled;
    tf->processingProperties.samplerate = getSampleRate();
    tf->InitScript(scriptfile.toStdString().c_str());
    setScriptInitialized(true);
    scriptFileName = scriptfile;
}

void JucePluginAudioProcessor::ReportLatency()
{
    setLatencySamples((!bypassed && scriptInitialized ? 44100 : 0));
}

void JucePluginAudioProcessor::setBypassed(bool b, bool notifyOtherInstances)
{
    if (bypassed == b)
        return;
    bypassed = b;
    ReportLatency();
    refreshEditorToggleButton();
}

void JucePluginAudioProcessor::setDbgOutEnable(bool b)
{
    if (debugOutputEnabled == b)
        return;

    debugOutputEnabled = b;
    if(tf && scriptInitialized)
        tf->debugOutputEnabled = b;
    refreshEditorToggleButton();
}

inline void JucePluginAudioProcessor::refreshBypassState(bool notifyOtherInstances)
{
    bool selected = autoBypass ? autoBypassState : userBypassState;
    if (bypassed != selected)
        setBypassed(selected, notifyOtherInstances);
}

inline void JucePluginAudioProcessor::setUserBypassState(bool b, bool notifyOtherInstances)
{
    if (userBypassState == b)
        return;
    userBypassState = b;
    if (notifyOtherInstances)
    {
        MidiTransformerIPCSync::MidiTransformerSyncedProperties p = SyncObject->properties;
        p.userBypassState = userBypassState;
        SyncObject->overwrite(p);
    }
    refreshBypassState(notifyOtherInstances);
}

inline void JucePluginAudioProcessor::setAutoBypassState(bool b, bool notifyOtherInstances)
{
    bool calledFromTimedBypass = false;
    if (bypassTimerThread->isThreadRunning() && bypassTimerThread->getThreadId() == juce::Thread::getCurrentThreadId())
    {
        calledFromTimedBypass = true;
    }
    if (bypassTimerThread->isThreadRunning() && !calledFromTimedBypass)
    {
        bypassTimerThread->stopThread(0);
    }

    if (autoBypassState != b)
    {
        autoBypassState = b;
        refreshBypassState(notifyOtherInstances);
    }
}

void JucePluginAudioProcessor::refreshEditorToggleButton()
{
    if (juce::AudioProcessorEditor* e = getActiveEditor())
        e->postCommandMessage(6); //refresh toggle buttons 
}

void JucePluginAudioProcessor::tellEditorClosePluginEditorWait()
{
    if (JucePluginAudioProcessorEditor* e = (JucePluginAudioProcessorEditor*)getActiveEditor())
    {
        e->SafeDelete(e->pluginWindow);
        //e->postCommandMessage(2);
        //waitCloseHostedEditor.wait();
    }
}


void JucePluginAudioProcessor::setScriptInitialized(bool b)
{
    scriptInitialized = b;
    ReportLatency();
}

void JucePluginAudioProcessor::luaFail()
{
}

void JucePluginAudioProcessor::SyncPropertiesUpdated(const MidiTransformerIPCSync::MidiTransformerSyncedProperties& properties)
{
    if (autoBypass != properties.autoBypass)
        setAutoBypass(properties.autoBypass, false);
    if (userBypassState != properties.userBypassState)
        setUserBypassState(properties.userBypassState, false);
    if (debugOutputEnabled)
    {
        debugMessages.add(juce::String("[SYNC] IPC Sync: autobypass = ") + juce::String((int)properties.autoBypass) + " , instances = " + juce::String(properties.instancesCount)+" , userbps = "+juce::String((int)properties.userBypassState));
        if (auto e = getActiveEditor())
            e->postCommandMessage(1);//refresh dbgout
    }
}

void JucePluginAudioProcessor::setAutoBypass(bool b,bool notifyOtherInstances)
{
    if (autoBypass == b)
        return;
    autoBypass = b;
    refreshBypassState();
    if (notifyOtherInstances)
    {
        MidiTransformerIPCSync::MidiTransformerSyncedProperties p = SyncObject->properties;
        p.autoBypass = autoBypass;
        SyncObject->overwrite(p);
    }
}

void JucePluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        // ..do something to the data...
    }

    //=====================================================================================
    if (autoBypass)
    {
        juce::AudioPlayHead* ph = getPlayHead();
        if (ph)
        {
            juce::AudioPlayHead::CurrentPositionInfo inf;
            ph->getCurrentPosition(inf);
            
            //host stopped playing.
            if (isPlayingLastState && !inf.isPlaying)
            {
                bypassTimerThread->startThread();
            }
            else if (!isPlayingLastState && inf.isPlaying)
            {
                setAutoBypassState(false);
            }
            isPlayingLastState = inf.isPlaying;
        }
    }

    int bufLen = buffer.getNumSamples();
    juce::MidiBuffer TransformedMidiMessage = midiMessages;

    if (scriptInitialized && ! bypassed)
    {
        //Pass midi messages to transformer
        tf->processMidi(midiMessages, bufLen);

        //Retrieving transformed midi messages
        tf->retrieveMidi(TransformedMidiMessage, bufLen);
        midiMessages.clear();
        midiMessages = TransformedMidiMessage;
    }

    if (pluginLoaded)
    {
        juce::AudioPluginInstance* plugin = hostedPlugin.get();
        plugin->prepareToPlay(getSampleRate(), bufLen);
        juce::MidiBuffer& selectedBuffer = scriptInitialized ? TransformedMidiMessage : midiMessages;

        for (auto e : selectedBuffer)
        {
            debugMessages.add(e.getMessage().getDescription());
        }
        plugin->processBlock(buffer, selectedBuffer);
    }

    if (debugOutputEnabled)
    {
        JucePluginAudioProcessorEditor* editor;
        if (editor = (JucePluginAudioProcessorEditor*)getActiveEditor())
        {
            bool refreshText = debugMessages.size();
            if (scriptInitialized && tf)
            {
                refreshText |= tf->debugMessages.size();
                while (tf->debugMessages.size())
                {
                    debugOutput += tf->debugMessages[0] + "\n";
                    tf->debugMessages.remove(0);
                }
            }
            while (debugMessages.size())
            {
                debugOutput += debugMessages[0] + "\n";
                debugMessages.remove(0);
            }
            editor->debugOutput = debugOutput;
            if(refreshText)
                editor->postCommandMessage(1);
        }
    }
}

JucePluginAudioProcessor::~JucePluginAudioProcessor()
{
    if (pluginLoaded)
        unloadPlugin();
    if (SyncObject)
    {
        MidiTransformerIPCSync::MidiTransformerSyncedProperties p = SyncObject->properties;
        p.instancesCount--;
        SyncObject->overwrite(p);
        SyncObject->signalThreadShouldExit();
        SyncObject->waitForThreadToExit(1000);
        delete SyncObject;
    }
    if (bypassTimerThread)
        delete bypassTimerThread;
}

juce::AudioProcessorEditor* JucePluginAudioProcessor::createEditor()
{
    auto e = new JucePluginAudioProcessorEditor(*this);
    e->setBounds(0, 0, 600, 300);
    return e;
}