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
	function PostKeyEvt(notename,mark,oct,velo,offset) 
		post_message(thispointer,NOTEON,GetMidiNoteNum(notename,mark,oct),velo,offset)
		post_message(thispointer,NOTEOFF,GetMidiNoteNum(notename,mark,oct),velo,offset+MsSmps(100))
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
end
init_framework()
---------------------------------------------------------------------------------------------
--DO NOT MODIFY CODE ABOVE
---------------------------------------------------------------------------------------------
--CUSTOMIZABLE SECTION START=================================================================
---------------------------------------------------------------------------------------------
function init_utils()
	--Compensation Constants (in milliseconds)
	lgt_fast 	= 	150
	lgt_medium 	= 	255
	lgt_gliss	=	255
	lgt_atkmed	=	102
	lgt_atkfast	=	60
	lgt_slow    =   300
	lgt_mleg		=	55
	spicc_atk		=	80
	stctsm_atk		=	80
	stac_atk		=	65
	sfz_atk			=	40
	sus_atk			=	45
	mleg_atk		= 	60

	keysw_lead      	=   10
	short_uniform_len 	= 	150

	trs_lgt_continue = MsSmps(3)

	lgt_overlap = 5
	mlgt_overlap = 15

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
		return 100<vel
	end
	function chk_vel_lgt_medium(vel)
		return 65<=vel and vel<=100
	end
	function chk_vel_lgt_slow(vel)
		return 20<vel and vel<65
	end
	function chk_vel_gliss(vel)
		return vel<=20
	end
	function chk_vel_lgt_fastatk(vel)
		return vel>100
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
	
	Note = {Type,NoteOnTime,OnTimeTransformed,NoteEndTime,EndTimeTransformed,LegatoLive,AssociatedTimer,NoteVelo,Prev,ToleranceState,ToleranceTimer}
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

function message_income(msgtype,control,value)
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
		NotesList[control] = {
			Type = NoteType.pending,
			NoteOnTime = 0,
			OnTimeTransformed = nil,
			NoteEndTime = nil,
			EndTimeTransformed = nil,
			LegatoLive = nil,
			AssociatedTimer = NewTimer(trs_short,control),
			NoteVelo = value,
			Prev = LastNote,
			ToleranceState = nil
		}
		DebugMessage("Timer: ",NotesList[control].AssociatedTimer)
		TimersList[NotesList[control].AssociatedTimer] ={
			AssociatedNote = control,
			TimerID = NotesList[control].AssociatedTimer,
			ValuePassed = control,
			Type = TimerType.length
		}
		LastNote = control
		DebugMessage(GetNoteName(control),"\tON ","Timer: ",TimersList[NotesList[control].AssociatedTimer].TimerID)
		--DebugMessage("Stored note : ",NotesList[control])
	elseif msgtype == TIMER then
		DebugMessage("timer hit : ",control)
		local timer = TimersList[control]
		if timer == nil then
			--DebugMessage("timer already abandoned, do nothing")
		else
			TimersList[control] = nil
			DebugMessage("timer removed.")

			--Long note process
			if timer.Type == TimerType.length then
				DebugMessage("Note ",GetNoteName(value)," type -> [long.PENDING]")
				NotesList[value].Type = NoteType.long.pending

				--Legato
				if LegatoModeOn then
					DebugMessage("Legato mode ON, type -> [long.legato.PENDING]")
					DebugMessage("PREV = ",GetNoteName(NotesList[value].Prev))
					NotesList[value].Type = NoteType.long.legato.pending
					NotesList[value].LegatoLive = true

					--Determine if the legato note is START or FOLLOW
					local isFollow = false
					if NotesList[NotesList[value].Prev] == nil then
						DebugMessage("Prev note REMOVED, type -> [long.legato.START]")
						NotesList[value].Type = NoteType.long.legato.start
					elseif NotesList[value].Prev == value then
						DebugMessage("Same note, type -> [long.legato.START]")
						NotesList[value].Type = NoteType.long.legato.start
					elseif NotesList[NotesList[value].Prev].LegatoLive then
						DebugMessage("LEGATO FOLLOW ",GetNoteName(NotesList[value].Prev)," -> ",GetNoteName(value))
						NotesList[value].Type = NoteType.long.legato.follow
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
							if chk_vel_lgt_fast(NotesList[value].NoteVelo) then
								compensation = lgt_fast
							elseif chk_vel_lgt_medium(NotesList[value].NoteVelo) then
								compensation = lgt_medium
							elseif chk_vel_lgt_slow(NotesList[value].NoteVelo) then
								compensation = lgt_slow
							elseif chk_vel_gliss(NotesList[value].NoteVelo) then
								compensation = lgt_gliss
							end
						end

						--Process Note
						PostMsg(NOTEON,value,NotesList[value].NoteVelo,NotesList[value].NoteOnTime - MsSmps(compensation))
						DebugMessage("LEGATO FOLLOW NOTE ON ",GetNoteName(value))
						PostMsg(NOTEOFF,NotesList[value].Prev,64,NotesList[value].NoteOnTime - MsSmps(compensation) +MsSmps(5))
						DebugMessage("TERMINATE PREV",GetNoteName(NotesList[value].Prev))
						if NotesList[NotesList[value].Prev] ~= nil then
							if NotesList[NotesList[value].Prev].ToleranceState == true then
								DebugMessage("PREV note in TOLERANCE STATE, REMOVE TIMER")
								TimersList[NotesList[NotesList[value].Prev].ToleranceTimer] = nil
							end
						end							
						NotesList[NotesList[value].Prev] = nil
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
						NotesList[value].Type = NoteType.long.legato.start
						keySwFunc(NotesList[value].NoteOnTime - MsSmps(compensation) - MsSmps(keysw_lead))
						PostMsg(NOTEON,value,NotesList[value].NoteVelo,NotesList[value].NoteOnTime - MsSmps(compensation))
						DebugMessage("LEGATO START NOTE ON ",GetNoteName(value))
					end
				--Sustain
				else
					DebugMessage("Legato mode OFF, type -> [long.SUSTAIN]")
					NotesList[value].Type = NoteType.long.sustain

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
					keySwFunc(NotesList[value].NoteOnTime - MsSmps(compensation) - MsSmps(keysw_lead))
					PostMsg(NOTEON,value,NotesList[value].NoteVelo,NotesList[value].NoteOnTime - MsSmps(compensation))
				end
			--ExitLegatoState process
			elseif timer.Type == TimerType.tolerance then
				if NotesList[timer.AssociatedNote] ~= nil and NotesList[timer.AssociatedNote].LegatoLive then
					NotesList[timer.AssociatedNote].LegatoLive = false;
					NotesList[timer.AssociatedNote].ToleranceState = false;
					DebugMessage(GetNoteName(timer.AssociatedNote)," TOLERANCE STATE END, EXIT LEGATO STATE")
					DebugMessage("NOTE END TIME ==============",NotesList[timer.AssociatedNote].NoteEndTime)
					PostMsg(NOTEOFF,timer.AssociatedNote,64,NotesList[timer.AssociatedNote].NoteEndTime)
					NotesList[timer.AssociatedNote] = nil
				else
					DebugMessage(GetNoteName(timer.AssociatedNote)," ALREADY removed, SKIP tolerance timer")
				end
			end
		end
	elseif msgtype == NOTEOFF then
		if(NotesList[control] == nil) then
			DebugMessage("Note already removed, skip.")
		else
			DebugMessage("Note ",GetNoteName(control),"\tOFF , Stored note: ",NotesList[control])
			NotesList[control].NoteEndTime = 0
			if NotesList[control].Type == NoteType.pending then
				TimersList[NotesList[control].AssociatedTimer] = nil
				DebugMessage("timer abandoned.")
				NotesList[control].Type = NoteType.short
				local noteLen = -NotesList[control].NoteOnTime
				DebugMessage("Note ",GetNoteName(control)," type -> short , length = ",noteLen)
				local shortCorrection = 0
				if chk_len_sfz(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [SFZ]")
					shortCorrection = sfz_atk
					switch_to_sfz(NotesList[control].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))	
				elseif chk_len_stac(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [STAC]")
					shortCorrection = stac_atk
					switch_to_stac(NotesList[control].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))
				elseif chk_len_stctsm(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [STCTSM]")
					shortCorrection = stctsm_atk
					switch_to_stctsm(NotesList[control].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))	
				elseif chk_len_spicc(noteLen) then
					--DebugMessage("length = ",noteLen," , type = [SPICC]")
					shortCorrection = spicc_atk
					switch_to_spicc(NotesList[control].NoteOnTime - MsSmps(shortCorrection)-MsSmps(keysw_lead))
				end
				PostMsg(NOTEON,control,NotesList[control].NoteVelo,NotesList[control].NoteOnTime - MsSmps(shortCorrection))
				PostMsg(NOTEOFF,control,64,NotesList[control].NoteOnTime - MsSmps(shortCorrection) + MsSmps(short_uniform_len))
				DebugMessage("[SHORT] Note ",GetNoteName(control)," removed from list")
				NotesList[control] = nil
			elseif NotesList[control].Type == NoteType.long.sustain then
				DebugMessage("Note ",GetNoteName(control)," [SUSTAIN] note off, length = ",-NotesList[control].NoteOnTime)
				NotesList[control] = nil
				DebugMessage("Note ",GetNoteName(control)," removed from list")
				PostMsg(NOTEOFF,control,value,0)
			elseif NotesList[control].Type == NoteType.long.legato.start or NotesList[control].Type == NoteType.long.legato.follow then
				DebugMessage("Note ",GetNoteName(control)," [LEGATO] note off, enter TOLERANCE STATE")
				NotesList[control].ToleranceState = true
				local tid = NewTimer(trs_lgt_continue+trs_short,control)
				NotesList[control].ToleranceTimer = tid
				TimersList[tid] = {
					AssociatedNote = control,
					TimerID = tid,
					ValuePassed = control,
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
