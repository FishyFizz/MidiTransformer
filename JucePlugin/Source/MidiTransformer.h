/*
  ==============================================================================

    MidiTransformer.h
    Created: 23 May 2021 3:49:47am
    Author:  Fizz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MidiTransformerNotifyMessage.h"
#include "../LUA/lauxlib.h"
#include <list>
#include <vector>
#include <algorithm>

#pragma comment(lib,"../../lua/lua.lib")

class MidiTransformer// : public juce::MessageListener
{
public:
    class TFScriptEvent
    {
    public:
        enum EvtType { NoteOnEvent, NoteOffEvent, MidiCCEvent, TimerEvent, NewBlock };
        int type;
        int control;
        int value;
        int countdownSmpls;
    };
    std::list<TFScriptEvent> notifyQueue;
    std::list<TFScriptEvent> resultQueue;

    juce::StringArray debugMessages;

    struct ProcessingProperties
    {
        int samplerate;
    };

    struct ScriptProperties
    {
        int requiredlatency;
    };

    lua_State* L;
    int LatencySmps;

    ProcessingProperties processingProperties;
    ScriptProperties scriptProperties;

    void processMidi(juce::MidiBuffer buf,int blockSize);
    void retrieveMidi(juce::MidiBuffer& mid,int blockSize);

    void InitScript(const char* luaFile);
    void SendMessage(const TFScriptEvent& msg);
    static TFScriptEvent Convert(juce::MidiMessage& const m, int samplePos);
    static juce::MidiMessage Convert(TFScriptEvent& const m); 
    void AdvanceTime(long samples);
    int AdvanceToNextNotifyEvent();
    void InsertToQueue(std::list<TFScriptEvent>& queue, TFScriptEvent& const e);

    short currTimerID = 1;

    //Callbacks and related utility for Lua
    static MidiTransformer* RetrieveThisPointer(lua_State* l);

    static int RequestCallback_Static(lua_State* l);
    int RequestTimer(int samples, int valueToPass);

    static int PostMessage_Static(lua_State* l);
    void PostMessage(int type, int control, int value,int offset);

    static int DebugMessage_Static(lua_State* l);
    void DebugMessage(const char* str);
    void AddDebugOutputHost(juce::String& const str);
    void AddDebugOutputScript(juce::String& const str);
    bool luaErrBypass = false;
    int deltaTime=0;
    //DEBUG 
    bool debugWatchBlock = false;
    long long debugLen = 0;
    long long advanceCounter = 0;
    bool debugOutputEnabled = true;
};

