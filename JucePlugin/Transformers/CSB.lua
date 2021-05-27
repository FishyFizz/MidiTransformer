function init_framework()
	--[[
	==============================================================================================
	IMPORTANT:
	These variables and functions are placeholders for lua syntax chk.
	When script is loaded by host, actual value and function address will be overwritten by host.
	]]
	samplerate = 44100
	thispointer = 0
	function request_callback_timer(thispointer, samples, valuetopass)
		local timerID = 0
		return timerID
	end
	function post_message(thispointer, type, control, value, offset)
	end
	function debug_message(thispointer,content)
	end
	function get_active_noteid_pitch(thispointer,content)
	end
	--============================================================================================
	---------------------------------------------------------------------------------------------
	--UTILITIES

	debug = false
	dbgout_tofile = false
	function EnableDebug()
		debug = true
		if dbgout_tofile then
			io.output(io.open("C:/Users/Fizz/Desktop/tfs.log","w"))
		end
	end
	function DbgExec(LuaStr)
		if debug then 
			return load(LuaStr)()
		end
	end
	function DebugMessage(...)
		local t = {...}
		local str = ''
		for i,v in pairs(t) do
			str = str..tostring(v)
		end
		if dbgout_tofile then
			io.write(str,'\n')
			io.flush()
		end
		debug_message(thispointer,str)
	end
	function ShowVarStr(var,varname)
		local str = ''
		if type(var) ~= 'table' then
			str = str..varname..' = '..var
			return str
		else
			str = str..varname..' = { '
			for i,v in pairs(var) do
				str = str..ShowVarStr(i,v)..' , '
			end
			return str..'}'
		end
	end
	NOTEON = 0
	NOTEOFF = 1
	CONTROLLER = 2
	TIMER = 3
	NEWBLOCK = 4
	noteNumTable = {['c']=0,['d']=2,['e']=4,['f']=5,['g']=7,['a']=9,['b']=11}
	marks = {['#']=1,['b']=-1,[' ']=0}
	noteNameTable = {'c','c#','d','d#','e','f','f#','g','g#','a','a#','b'}

	function at(str,n)
		return string.char(string.byte(str,n))
	end
	function EnableStringIndexing()
		local mt = getmetatable(string)
	end
	function GetMidiNoteNum(name,sharpflat,octave)
		return noteNumTable[name] + marks[sharpflat] + (octave*12+24)
	end
	function GetNoteName(num)
		return noteNameTable[(num%12+1)]..(math.floor(num/12)-2)
	end
	function MsSmps(ms)
		return math.floor(ms*samplerate/1000)
	end
	function SmpsMs(smps)
		return math.floor(smps*1000/samplerate)
	end
	
	ksw_len		=	MsSmps(1)
	function PostKeyEvt(notename,mark,oct,velo,offset) 
		post_message(thispointer,NOTEON,GetMidiNoteNum(notename,mark,oct),velo,offset)
		post_message(thispointer,NOTEOFF,GetMidiNoteNum(notename,mark,oct),velo,offset+ksw_len)
	end
	function PostControllerEvt(cc,val,offset)
		post_message(thispointer,CONTROLLER,cc,val,offset)
	end
	function BoolController(val)
		return val>=64
	end
	function NewTimer(smps,customval)
		return request_callback_timer(thispointer, smps, customval)
	end
	function PostMsg(type, control, value, offset)
		post_message(thispointer,type,control,value,offset)
	end
	function GetActiveNoteIdByPitch(p)
		return get_active_noteid_pitch(thispointer,p)
	end
end
init_framework()
---------------------------------------------------------------------------------------------
--DO NOT MODIFY CODE ABOVE
---------------------------------------------------------------------------------------------
--CUSTOMIZABLE SECTION START=================================================================
---------------------------------------------------------------------------------------------
function init_utils()
	--Compensation Constants (in milliseconds)
	lgt_fast 	= 	156
	lgt_medium 	= 	185
	lgt_atkfast	=	50
	lgt_mleg		=	101
	spicc_atk		=	75
	stctsm_atk		=	55
	stac_atk		=	75
	sfz_atk			=	70
	sus_atk			=	70
	mleg_atk		= 	70

	lgt_release		=	80

	keysw_lead      	=   10
	short_uniform_len 	= 	150

	trs_lgt_continue = MsSmps(3)

	lgt_overlap = 5
	mlgt_overlap = 10

	trs_stctsm	=	MsSmps(40)
	trs_stac	= 	trs_stctsm*2
	trs_sfz 	=	trs_stctsm*3
	trs_short 	= 	trs_stctsm*4

	function chk_len_spicc(smps) 
		return smps<trs_stctsm
	end
	function chk_len_stctsm(smps)
		return trs_stctsm<=smps and smps<trs_stac
	end
	function chk_len_stac(smps)
		return trs_stac<=smps and smps<trs_sfz
	end
	function chk_len_sfz(smps)
		return trs_sfz<=smps and smps<trs_short
	end

	function chk_vel_lgt_fast(vel)
		return 64<=vel
	end
	function chk_vel_lgt_medium(vel)
		return vel<=63
	end
	---------------------------------------------------------------------------------------------
	--Functions and States
	function switch_to_spicc(eventoffset) 
		PostKeyEvt('f',' ','0',127,eventoffset)
		PostControllerEvt(1,1,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_stctsm(eventoffset) 
		PostKeyEvt('f',' ','0',127,eventoffset)
		PostControllerEvt(1,40,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_stac(eventoffset)
		PostKeyEvt('f',' ','0',127,eventoffset)
		PostControllerEvt(1,80,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_sfz(eventoffset)
		PostKeyEvt('f',' ','0',127,eventoffset)
		PostControllerEvt(1,127,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_lgt(eventoffset)
		PostKeyEvt('c',' ','0',127,eventoffset)
		PostKeyEvt('b','b','0',127,eventoffset+MsSmps(1))
	end
	function switch_to_sustain(eventoffset)
		PostKeyEvt('c',' ','0',127,eventoffset)
		PostKeyEvt('b','b','0',1,eventoffset+MsSmps(1))
	end
	function switch_to_mlgt(eventoffset)
		PostKeyEvt('f','#','0',127,eventoffset)
		PostKeyEvt('b','b','0',127,eventoffset+MsSmps(1))
	end
	function switch_to_marcato(eventoffset)
		PostKeyEvt('f','#','0',127,eventoffset)
		PostKeyEvt('b','b','0',1,eventoffset+MsSmps(1))
	end

	NoteType = {
		pending = 1,
		short = 2,
		long = {
			pending=3,
			sustain = 4,
			legato = {
				pending = 5,
				start = 6,
				follow = 7
			}
		}
	}
	TimerType = {length = 1, tolerance = 2}
	
	Note = {Type,NoteOnTime,OnTimeTransformed,NoteEndTime,EndTimeTransformed,LegatoLive,AssociatedTimer,NoteVelo,Prev,ToleranceState,ToleranceTimer,NoteNum}
	Timer = {AssociatedNote,TimerID,ValuePassed,Type}
	
	NotesList = {}
	TimersList = {}
	
	LegatoModeOn = true
	LastNote = -1
	
	EnableSwitchCtrl 	= 4
	LegatoSwitchCtrl	= 5
	MarcatoSwitchCtrl	= 6
	
	TransformEnabled	= true
	MarcatoModeOn		= false	
end
--===========================================================================================

function init_script()
	init_utils()
	EnableDebug()
	DebugMessage("Script Initialized. SR=",samplerate)
end

function message_income(msgtype,control,value,assignid)
	DebugMessage("message_income,",msgtype,",",control,",",value)

	if msgtype == CONTROLLER --[[and control is one of the switches]] then
		--Process switches
		if control == EnableSwitchCtrl then
			TransformEnabled = BoolController(value)
			DebugMessage("ENABLE STATE = ",TransformEnabled)
			--clean up when bypassed
			if not TransformEnabled then
				NotesList = {}
				TimersList = {}
				LastNote = -1
			end
		elseif control == LegatoSwitchCtrl then
			LegatoModeOn = BoolController(value)
			DebugMessage("LEGATO MODE = ",LegatoModeOn)
		elseif control == MarcatoSwitchCtrl then
			MarcatoModeOn = BoolController(value)
			DebugMessage("MARCATO MODE = ",MarcatoModeOn)
		end
		return
	end --return after switch is processed

	if not TransformEnabled then --Bypassed, forward NoteOn NoteOff and Controllers
		if msgtype == CONTROLLER or msgtype == NOTEON or msgtype == NOTEOFF then
			PostMsg(msgtype,control,value)
		end
		return
	end --return after forwarding message

	if msgtype == CONTROLLER then
		--Bypass all controllers other than switches
		PostMsg(msgtype,control,value,0)
	elseif msgtype == NOTEON then
		NotesList[assignid] = {
			Type = NoteType.pending,
			NoteOnTime = 0,
			OnTimeTransformed = nil,
			NoteEndTime = nil,
			EndTimeTransformed = nil,
			LegatoLive = nil,
			AssociatedTimer = NewTimer(trs_short,control),
			NoteVelo = value,
			Prev = LastNote,
			ToleranceState = nil,
			NoteNum = control
		}
		DebugMessage("Timer: ",NotesList[assignid].AssociatedTimer)
		TimersList[NotesList[assignid].AssociatedTimer] ={
			AssociatedNote = assignid,
			TimerID = NotesList[assignid].AssociatedTimer,
			ValuePassed = assignid,
			Type = TimerType.length
		}
		LastNote = assignid
		DebugMessage(GetNoteName(control),"\tON ","Timer: ",TimersList[NotesList[assignid].AssociatedTimer].TimerID, "NoteID = ",assignid)
		--DebugMessage("Stored note : ",NotesList[control])
	elseif msgtype == TIMER then
		DebugMessage("timer hit : ",control)
		local timer = TimersList[control]
		if timer == nil then
			--DebugMessage("timer already abandoned, do nothing")
		else
			local currNoteId = timer.AssociatedNote
			TimersList[control] = nil
			DebugMessage("timer removed.")

			--Long note process
			if timer.Type == TimerType.length then
				DebugMessage("Note ",GetNoteName(NotesList[currNoteId].NoteNum)," type -> [long.PENDING]")
				NotesList[currNoteId].Type = NoteType.long.pending

				--Legato
				if LegatoModeOn then
					DebugMessage("Legato mode ON, type -> [long.legato.PENDING]")
					DebugMessage("PREV = ",GetNoteName(NotesList[currNoteId].Prev))
					NotesList[currNoteId].Type = NoteType.long.legato.pending
					NotesList[currNoteId].LegatoLive = true

					--Determine if the legato note is START or FOLLOW
					local isFollow = false
					if NotesList[NotesList[currNoteId].Prev] == nil then
						DebugMessage("Prev note REMOVED, type -> [long.legato.START]")
						NotesList[currNoteId].Type = NoteType.long.legato.start
					elseif NotesList[currNoteId].Prev == value then
						DebugMessage("Same note, type -> [long.legato.START]")
						NotesList[currNoteId].Type = NoteType.long.legato.start
					elseif NotesList[NotesList[currNoteId].Prev].LegatoLive then
						DebugMessage("LEGATO FOLLOW ",GetNoteName(NotesList[currNoteId].Prev)," -> ",GetNoteName(value))
						NotesList[currNoteId].Type = NoteType.long.legato.follow
						isFollow = true
					end

					--Process Follow note
					if isFollow then
						--Process follow note

						--Select legato transition length
						local compensation = 0
						local overlap = 0
						if MarcatoModeOn then
							compensation = lgt_mleg
							overlap = mlgt_overlap
						else
							overlap = lgt_overlap
							if chk_vel_lgt_fast(NotesList[currNoteId].NoteVelo) then
								compensation = lgt_fast
							elseif chk_vel_lgt_medium(NotesList[currNoteId].NoteVelo) then
								compensation = lgt_medium
							end
						end

						--Process Note
						PostMsg(NOTEON,NotesList[currNoteId].NoteNum,NotesList[currNoteId].NoteVelo,NotesList[currNoteId].NoteOnTime - MsSmps(compensation))
						DebugMessage("LEGATO FOLLOW NOTE ON ",GetNoteName(value))
						PostMsg(NOTEOFF,NotesList[NotesList[currNoteId].Prev].NoteNum,64,NotesList[currNoteId].NoteOnTime - MsSmps(compensation) +MsSmps(overlap))
						DebugMessage("TERMINATE PREV",GetNoteName(NotesList[NotesList[currNoteId].Prev].NoteNum))
						if NotesList[NotesList[currNoteId].Prev] ~= nil then
							if NotesList[NotesList[currNoteId].Prev].ToleranceState == true then
								DebugMessage("PREV note in TOLERANCE STATE, REMOVE TIMER")
								TimersList[NotesList[NotesList[currNoteId].Prev].ToleranceTimer] = nil
							end
						end							
						NotesList[NotesList[currNoteId].Prev] = nil
						DebugMessage("PREV note REMOVED")
					--Process Start Note
					else
						local keySwFunc = nil
						local compensation = nil
						--Select Compensation and Keyswitch
						if MarcatoModeOn then
							keySwFunc = switch_to_mlgt
							compensation = mleg_atk
						else
							keySwFunc = switch_to_lgt
							compensation = lgt_atkfast
						end

						--Process note
						NotesList[currNoteId].Type = NoteType.long.legato.start
						keySwFunc(NotesList[currNoteId].NoteOnTime - MsSmps(compensation) - MsSmps(keysw_lead))
						PostMsg(NOTEON,NotesList[currNoteId].NoteNum,NotesList[currNoteId].NoteVelo,NotesList[currNoteId].NoteOnTime - MsSmps(compensation))
						DebugMessage("LEGATO START NOTE ON ",GetNoteName(NotesList[currNoteId].NoteNum))
					end
				--Sustain
				else
					DebugMessage("Legato mode OFF, type -> [long.SUSTAIN]")
					NotesList[currNoteId].Type = NoteType.long.sustain

					local keySwFunc = nil
					local compensation = 0
					--Select technique and compensation
					if MarcatoModeOn then
						keySwFunc = switch_to_marcato
						compensation = mleg_atk
					else
						keySwFunc = switch_to_sustain
						compensation = sus_atk
					end
					--Process sustain note
					keySwFunc(NotesList[currNoteId].NoteOnTime - MsSmps(compensation) - MsSmps(keysw_lead))
					PostMsg(NOTEON,NotesList[currNoteId].NoteNum,NotesList[currNoteId].NoteVelo,NotesList[currNoteId].NoteOnTime - MsSmps(compensation))
				end
			--ExitLegatoState process
			elseif timer.Type == TimerType.tolerance then
				DebugMessage("ToleranceTimer , noteid = ",currNoteId)
				if NotesList[currNoteId] ~= nil then
					NotesList[currNoteId].LegatoLive = false;
					NotesList[currNoteId].ToleranceState = false;
					DebugMessage(GetNoteName(NotesList[currNoteId].NoteNum)," TOLERANCE STATE END, EXIT LEGATO STATE")
					DebugMessage("NOTE END TIME ==============",NotesList[currNoteId].NoteEndTime)
					PostMsg(NOTEOFF,NotesList[currNoteId].NoteNum,64,NotesList[currNoteId].NoteEndTime-MsSmps(lgt_release))
					NotesList[currNoteId] = nil
				else
					DebugMessage("ALREADY removed, SKIP tolerance timer")
				end
			end
		end
	elseif msgtype == NOTEOFF then
		if(NotesList[assignid] == nil) then
			DebugMessage("Note already removed, skip.")
		else
			DebugMessage("Note ",GetNoteName(control),"\tOFF , Stored note: ",NotesList[assignid])
			NotesList[assignid].NoteEndTime = 0
			if NotesList[assignid].Type == NoteType.pending then
				TimersList[NotesList[assignid].AssociatedTimer] = nil
				DebugMessage("timer abandoned.")
				NotesList[assignid].Type = NoteType.short
				local noteLen = -NotesList[assignid].NoteOnTime
				DebugMessage("Note ",GetNoteName(control)," type -> short , length = ",noteLen)
				local shortCorrection = 0
				if chk_len_sfz(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [SFZ]")
					shortCorrection = sfz_atk
					switch_to_sfz(NotesList[assignid].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))	
				elseif chk_len_stac(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [STAC]")
					shortCorrection = stac_atk
					switch_to_stac(NotesList[assignid].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))
				elseif chk_len_stctsm(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [STCTSM]")
					shortCorrection = stctsm_atk
					switch_to_stctsm(NotesList[assignid].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))	
				elseif chk_len_spicc(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [SPICC]")
					shortCorrection = spicc_atk
					switch_to_spicc(NotesList[assignid].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))
				end
				PostMsg(NOTEON,control,NotesList[assignid].NoteVelo,NotesList[assignid].NoteOnTime - MsSmps(shortCorrection))
				PostMsg(NOTEOFF,control,64,NotesList[assignid].NoteOnTime - MsSmps(shortCorrection) + MsSmps(short_uniform_len))
				DebugMessage("[SHORT] Note ",GetNoteName(control)," removed from list")
				NotesList[assignid] = nil
			elseif NotesList[assignid].Type == NoteType.long.sustain then
				DebugMessage("Note ",GetNoteName(control)," [SUSTAIN] note off, length = ",-NotesList[assignid].NoteOnTime)
				NotesList[assignid] = nil
				DebugMessage("Note ",GetNoteName(control)," removed from list")
				PostMsg(NOTEOFF,control,value,0)
			elseif NotesList[assignid].Type == NoteType.long.legato.start or NotesList[assignid].Type == NoteType.long.legato.follow then
				DebugMessage("Note ",GetNoteName(control)," [LEGATO] note off, enter TOLERANCE STATE")
				NotesList[assignid].ToleranceState = true
				local tid = NewTimer(trs_lgt_continue+trs_short,assignid)
				NotesList[assignid].ToleranceTimer = tid
				TimersList[tid] = {
					AssociatedNote = assignid,
					TimerID = tid,
					ValuePassed = assignid,
					Type = TimerType.tolerance
				}
				DebugMessage("Tolerance state timer added.")
				--NotesList[control] = nil
				--PostMsg(NOTEOFF,control,value,0)
			end
		end
	end
end

function advance_time(samples)
	--PLACE CODE HERE
	--DebugMessage("\t\t\t\tAdvance time")
	for notenum,notedata in pairs(NotesList) do
		local fields = {'NoteOnTime','NoteEndTime','OnTimeTransformed','EndTimeTransformed'}
		for i,field in pairs(fields) do
			if notedata[field] ~= nil then
				notedata[field] = notedata[field] - samples
			end
		end
	end
end
