/*
  ==============================================================================

    MidiTransformer.cpp
    Created: 23 May 2021 3:49:47am
    Author:  Fizz

  ==============================================================================
*/

#include "MidiTransformer.h"
#include "../LUA/lauxlib.h"
#include "../LUA/lualib.h"

#define STR(x) (juce::String(x))

short MidiTransformer::AssignId(short& Prev)
{
    short ret = Prev;
    Prev++;
    if (Prev == SHRT_MAX)
        Prev = 1;
    return Prev;
}

MidiTransformer::MidiTransformer()
{
    ActiveNoteIdTable.fill(0);
}

void MidiTransformer::processMidi(juce::MidiBuffer buf,int blockSize)
{
    //====
    //if (debugWatchBlock)
    //{
    //    AddDebugOutputHost("BLOCK len = " + STR(blockSize));
    //}
    //====
    if (luaErrBypass)
        return;

    int n = buf.getNumEvents();
    juce::MidiBufferIterator ite = buf.begin();
    for (int i = 0; i < n; i++)
    {
        juce::MidiMessage msg = (*ite).getMessage();
        InsertToQueue(notifyQueue,Convert(msg,(*ite).samplePosition));
//============================================================================
        if (msg.isNoteOn())
        {
            AddDebugOutputHost("NOTE ON @ " + STR((*ite).samplePosition) + "/" + STR(blockSize));
            debugWatchBlock = true;
        }
        if (msg.isNoteOff())
        {
            AddDebugOutputHost("NOTE OFF @ " + STR((*ite).samplePosition) + "/" + STR(blockSize));
            debugWatchBlock = false;
        }
//============================================================================
        ite++;
    }
    
    advanceCounter = 0;
    int samplesAdvanced = 0;
    while (notifyQueue.size() && notifyQueue.begin()->countdownSmpls+samplesAdvanced<=blockSize)
    {
        samplesAdvanced += AdvanceToNextNotifyEvent();
        // Do NOT pass *notifyQueue.begin() to SendMessage directly and pop later
        // the events added by script may be deleted, and the event shoul be deleted is left
        TFScriptEvent e = *notifyQueue.begin();
        notifyQueue.pop_front();
        if (e.type == TFScriptEvent::EvtType::NoteOnEvent)
        {
            if (ActiveNoteIdTable[e.control]) //note currently active, this NoteOn event is duplicate. Ignore.
                continue;
            else
            {
                e.assignObjectId = AssignId(NoteIdAssign);
                ActiveNoteIdTable[e.control] = e.assignObjectId;
            }
        }
        else if (e.type == TFScriptEvent::EvtType::NoteOffEvent)
        {
            if (!ActiveNoteIdTable[e.control]) //note currently inactive, this NoteOff event is duplicate. Ignore.
                continue;
            else
            {
                e.assignObjectId = ActiveNoteIdTable[e.control];
                ActiveNoteIdTable[e.control] = 0; //set note inactive
            }
        }
        SendMessage(e);
    }
    AdvanceTime(blockSize - samplesAdvanced);

    //if(debugWatchBlock)
    //    AddDebugOutputHost("ADVANCE TIME " + STR(advanceCounter) + "/" + STR(blockSize));
}

void MidiTransformer::retrieveMidi(juce::MidiBuffer& mid,int blockSize)
{
    mid.clear();
    while (resultQueue.size())
    {
        auto curr = (*resultQueue.begin());
        if (curr.countdownSmpls >= blockSize)
            return;
        if (curr.type != curr.NewBlock && curr.type != curr.TimerEvent)
        {
            mid.addEvent(Convert(curr),curr.countdownSmpls);
            resultQueue.pop_front();
        }  
    }
}

void MidiTransformer::InitScript(const char* luaFile)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    
    //Load transformer script
    PrepareSafeCall();
    luaL_loadfile(L, luaFile);
    if (SafeCall(0, 0) == 0) PostSafeCallSuccess();

//  samplerate = [samplerate]
    lua_pushinteger(L, processingProperties.samplerate);
    lua_setglobal(L, "samplerate");

//  thispointer = [this]
    lua_pushinteger(L, (long long)this);
    lua_setglobal(L, "thispointer");

//  register callback
    lua_register(L, "request_callback_timer", RequestCallback_Static);
    lua_register(L, "post_message", PostMessage_Static);
    lua_register(L, "debug_message", DebugMessage_Static);

//  void init_script()
    PrepareSafeCall();
    lua_getglobal(L, "init_script");
    if (SafeCall(0, 0) == 0) PostSafeCallSuccess();
}

void MidiTransformer::SendMessage(const TFScriptEvent& msg)
{
//  (void) message_income(msgtype, control, value)
    PrepareSafeCall();
    lua_getglobal(L, "message_income");
    lua_pushinteger(L, msg.type);
    lua_pushinteger(L, msg.control);
    lua_pushinteger(L, msg.value);
    lua_pushinteger(L, msg.assignObjectId);
    if (SafeCall(4, 0) == 0) PostSafeCallSuccess();
}

MidiTransformer::TFScriptEvent MidiTransformer::Convert(juce::MidiMessage& const m, int samplePos, int ObjId )
{
    if (m.isNoteOn())
        return TFScriptEvent{ TFScriptEvent::EvtType::NoteOnEvent,m.getNoteNumber(),m.getVelocity(), samplePos ,ObjId};
    else if (m.isNoteOff())
        return TFScriptEvent{ TFScriptEvent::EvtType::NoteOffEvent,m.getNoteNumber(),m.getVelocity(), samplePos ,ObjId };
    else if (m.isController())
        return TFScriptEvent{ TFScriptEvent::EvtType::MidiCCEvent,m.getControllerNumber(),m.getControllerValue(), samplePos ,ObjId };
    return TFScriptEvent{ -1,0,0,0 ,0};
}

juce::MidiMessage MidiTransformer::Convert(TFScriptEvent& const m)
{
    juce::MidiMessage result;
    if (m.type == m.MidiCCEvent)
        result = juce::MidiMessage::controllerEvent(1, m.control, m.value);
    else if (m.type == m.NoteOnEvent)
        result = juce::MidiMessage::noteOn(1, m.control, juce::uint8(m.value));
    else if (m.type == m.NoteOffEvent)
        result = juce::MidiMessage::noteOff(1, m.control, juce::uint8(m.value));

    return result;
}


void MidiTransformer::AdvanceTime(long samples)
{
    advanceCounter += samples;
//  void advance_time(samples)
    PrepareSafeCall();
    lua_getglobal(L, "advance_time");
    lua_pushinteger(L, samples);
    if (SafeCall(1, 0) == 0) PostSafeCallSuccess();

    for (TFScriptEvent& e : notifyQueue) e.countdownSmpls -= samples;
    for (TFScriptEvent& e : resultQueue) e.countdownSmpls -= samples;
}

int MidiTransformer::AdvanceToNextNotifyEvent()
{
    if (notifyQueue.size() == 0) return 0;
    int ret = notifyQueue.begin()->countdownSmpls;
    AdvanceTime(notifyQueue.begin()->countdownSmpls);
    return ret;
}



// timerID request_callback_timer(thispointer,samples,valuetopass)
int MidiTransformer::RequestCallback_Static(lua_State* l)
{
    int n = lua_gettop(l); //should be 3

    MidiTransformer* pthis = RetrieveThisPointer(l);

    int samples = lua_tointeger(l, 2);
    int valueToPass = lua_tointeger(l, 3);

    int timerID = pthis->RequestTimer(samples,valueToPass);
    lua_pushinteger(l, timerID);
    return 1;
}

void MidiTransformer::InsertToQueue(std::list<TFScriptEvent>& queue,TFScriptEvent& const e)
{
    if (e.type == -1) //not a valid message
        return;

    for (auto ite = queue.begin(); ite != queue.end(); ite++)
    {
        if ((*ite).countdownSmpls >= e.countdownSmpls)
        {   
            //midi cc event have higer priority
            while (ite != queue.end()&&
                (*ite).countdownSmpls == e.countdownSmpls &&
                (*ite).type == TFScriptEvent::EvtType::MidiCCEvent)
            {
                ite++;
            }

            queue.insert(ite, e);
            return;
        }
    }
    queue.push_back(e);
}

void MidiTransformer::PrepareSafeCall()
{
    lua_getglobal(L, "ShowVarStr");
}

int MidiTransformer::SafeCall(int args, int rets)
{
    int err = lua_pcall(L, args, rets, 0);
    if (err)
    {
        luaErrBypass = true;
        lua_pushstring(L, "ErrorObj");
        lua_call(L, 2, 1);
        juce::String errmsg = lua_tostring(L, 1);
        lua_pop(L, 1); //Stack cleared
        juce::NativeMessageBox::showMessageBoxAsync(juce::AlertWindow::AlertIconType::InfoIcon, "ERROR", "Lua script failed!");
        juce::NativeMessageBox::showMessageBoxAsync(juce::AlertWindow::AlertIconType::InfoIcon, "ERROR", errmsg);
    }
    return err;
}

void MidiTransformer::PostSafeCallSuccess()
{
    lua_pop(L, 1);
}

// void post_message(pthis,type,control,value,offset)
int MidiTransformer::PostMessage_Static(lua_State* l)
 {
    int n = lua_gettop(l); //should be 5

    MidiTransformer* pthis = RetrieveThisPointer(l);

    int type = lua_tointeger(l, 2);
    int control= lua_tointeger(l, 3);
    int value= lua_tointeger(l, 4);
    int offset = lua_tointeger(l, 5);

    pthis->PostMessage(type, control, value, offset);
    return 0;
}

void MidiTransformer::PostMessage(int type, int control, int value, int offset)
{
    TFScriptEvent e = { type,control,value,LatencySmps + offset };
    InsertToQueue(resultQueue, e);
}

int MidiTransformer::DebugMessage_Static(lua_State* l)
{
    int n = lua_gettop(l); //should be 2
    MidiTransformer* pthis = RetrieveThisPointer(l);
    const char* str = lua_tostring(l, 2);
    
    pthis->DebugMessage(str);
    return 0;
}

void MidiTransformer::DebugMessage(const char* str)
{
    AddDebugOutputScript(STR(str));
}

void MidiTransformer::AddDebugOutputHost(juce::String& const str)
{
    if (!debugOutputEnabled) return;
    debugMessages.add(STR("                   [host] ") + STR(str));
}

void MidiTransformer::AddDebugOutputScript(juce::String& const str)
{
    if (!debugOutputEnabled) return;
    debugMessages.add(STR("[script] ") + STR(str));
}

int MidiTransformer::RequestTimer(int samples, int valueToPass)
{
    AddDebugOutputHost("request timer for "+STR(samples)+" samples");
    int selectedID = AssignId(TimerIdAssign);
    TFScriptEvent e = TFScriptEvent{ TFScriptEvent::EvtType::TimerEvent,selectedID,valueToPass,samples};
    InsertToQueue(notifyQueue,e);
    return selectedID;
}

MidiTransformer* MidiTransformer::RetrieveThisPointer(lua_State* l)
{
    long long tmp;
    MidiTransformer* pthis;
    tmp = lua_tointeger(l, 1);
    pthis = (MidiTransformer*)((void*)tmp);
    return pthis;
}