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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void JucePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//CUSTOMIZED====================================================================

bool JucePluginAudioProcessor::initPlugin(juce::PluginDescription& const desc)
{
    if (pluginLoaded)
    {
        pluginLoaded = false;
        hostedPlugin.release();
    }
        

    juce::VST3PluginFormat f3;
    hostedPlugin = f3.createInstanceFromDescription(desc, getSampleRate(), 512);
    if (!hostedPlugin)
    {
        juce::VSTPluginFormat f;
        hostedPlugin = f.createInstanceFromDescription(desc, getSampleRate(), 512);
    }
    if (!hostedPlugin)
        return false;
    if (auto e = getActiveEditor())
        e->postCommandMessage(5);
    return true;
}

void JucePluginAudioProcessor::unloadPlugin()
{
    if (!pluginLoaded)
        return;
    if (auto e = getActiveEditor())
        e->postCommandMessage(2); //release hosted window
    hostedPlugin.release();
    pluginLoaded = false;
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
}

void JucePluginAudioProcessor::ReportLatency()
{
    setLatencySamples((!bypassed && scriptInitialized ? 44100 : 0));
}

void JucePluginAudioProcessor::setBypassed(bool b)
{
    bypassed = b;
    ReportLatency();
}

void JucePluginAudioProcessor::setScriptInitialized(bool b)
{
    scriptInitialized = b;
    ReportLatency();
}

void JucePluginAudioProcessor::luaFail()
{
}

void JucePluginAudioProcessor::setAutoBypass(bool b)
{
    autoBypass = b;
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
            setBypassed(!inf.isPlaying);
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

        if (debugOutputEnabled)
        {
            bool refreshText = tf->debugMessages.size();
            while (tf->debugMessages.size())
            {
                debugOutput += tf->debugMessages[0] + "\n";
                tf->debugMessages.remove(0);
            }
            JucePluginAudioProcessorEditor* editor;
            if ((editor = (JucePluginAudioProcessorEditor*)getActiveEditor()) && refreshText)
            {
                editor->debugOutput = debugOutput;
                editor->postCommandMessage(1);
            }
        }
    }

    if (pluginLoaded)
    {
        juce::AudioPluginInstance* plugin = hostedPlugin.get();
        plugin->prepareToPlay(getSampleRate(), bufLen);
        plugin->processBlock(buffer,  scriptInitialized?TransformedMidiMessage:midiMessages);
    }
}

JucePluginAudioProcessor::~JucePluginAudioProcessor()
{
    if (pluginLoaded)
        unloadPlugin();
}

juce::AudioProcessorEditor* JucePluginAudioProcessor::createEditor()
{
    auto e = new JucePluginAudioProcessorEditor(*this);
    e->setBounds(0, 0, 500, 700);
    return e;
}