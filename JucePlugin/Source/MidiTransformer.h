/*
  ==============================================================================

    MidiTransformer.h
    Created: 23 May 2021 3:49:47am
    Author:  Fizz

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../LUA/lauxlib.h"
#include <list>
#include <vector>
#include <array>
#include <algorithm>

#pragma comment(lib,"../../lua/lua.lib")

class MidiTransformer// : public juce::MessageListener
{
public:

    MidiTransformer();

    class TFScriptEvent
    {
    public:
        enum EvtType { NoteOnEvent, NoteOffEvent, MidiCCEvent, TimerEvent, NewBlock };
        int type;
        int control;
        int value;
        int countdownSmpls;
        int assignObjectId;
    };
    std::array<int, 128> ActiveNoteIdTable;

    //Offers note trimming in a first-in-first-out method
    std::array<int, 128> NoteOverlapCounter;

    short NoteIdAssign = 1;
    short TimerIdAssign = 1;

    short AssignId(short& Prev);

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
    static TFScriptEvent Convert(juce::MidiMessage& const m, int samplePos, int ObjId = 0);
    static juce::MidiMessage Convert(TFScriptEvent& const m); 
    void AdvanceTime(long samples);
    int AdvanceToNextNotifyEvent();
    void InsertToQueue(std::list<TFScriptEvent>& queue, TFScriptEvent& const e);

    bool autoBypassSwitch = true;
    void setAutoBypass(bool b);
    //Callbacks and related utility for Lua
    void PrepareSafeCall();
    int SafeCall(int args, int rets);
    void PostSafeCallSuccess();

    static MidiTransformer* RetrieveThisPointer(lua_State* l);

    static int RequestCallback_Static(lua_State* l);
    int RequestTimer(int samples, int valueToPass);

    static int PostMessage_Static(lua_State* l);
    void PostMessage(int type, int control, int value,int offset);

    static int GetActiveNoteIdByPitch_Static(lua_State* l);
    int GetActiveNoteIdByPitch(int pitch);

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

