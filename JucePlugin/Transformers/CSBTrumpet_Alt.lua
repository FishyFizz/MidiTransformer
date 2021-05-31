function init_framework()
	--[[
	==============================================================================================
	IMPORTANT:
	These variables and functions are placeholders for lua syntax chk.
	When script is loaded by host, actual value and function address will be overwritten by host.
	]]
	samplerate = 44100
	thispointer = 0
	
	LUAINTMAX = 9223372036854775807
	
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
	function StrFormatLen(str,len)
		local tmp = tostring(str)
		if len ~= nil then
			return tmp..string.rep(' ',math.max(len-#tmp,0))
		else
			return tmp
		end
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
	function _GetMidiNoteNum(name,sharpflat,octave)
		return noteNumTable[name] + marks[sharpflat] + (octave*12+24)
	end
	function NoteNum(name)
		name = string.lower(name)
		local ch = at(name,1)
		local sharpflat = ' '
		local oct = ''
		local tmp = at(name,2)

		local octIndex
		if tmp == '#' or tmp == 'b' then
			sharpflat = tmp
			octIndex = 3
		else
			octIndex = 2
		end

		if at(name,octIndex) == '-' then
			oct = -at(name,octIndex+1)
		else
			oct = at(name,octIndex)
		end

		return _GetMidiNoteNum(ch,sharpflat,oct)
	end
	function GetNoteName(num, formatlen)
		local tmp = noteNameTable[(num%12+1)]..(math.floor(num/12)-2)
		return StrFormatLen(tmp, formatlen)
	end
	function MsSmps(ms)
		return math.floor(ms*samplerate/1000)
	end
	function SmpsMs(smps)
		return math.floor(smps*1000/samplerate)
	end
	
	ksw_len		=	MsSmps(1)
	function PostKeyEvt(notename,velo,offset) 
		post_message(thispointer,NOTEON,NoteNum(notename),velo,offset)
		post_message(thispointer,NOTEOFF,NoteNum(notename),velo,offset+ksw_len)
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

	--Legato Transition Constants
	lgt_fast 			= 	156
	lgt_medium 			= 	185
	lgt_atkfast			=	50
	lgt_mleg			=	101
	--Legato Follow Tolerance
	trs_lgt_continue 	= MsSmps(3)
	--Legato Overlap Constants
	lgt_overlap 		= 5
	mlgt_overlap		= 10

	--Attack Compensation Constants
	spicc_atk			=	75
	stctsm_atk			=	55
	stac_atk			=	75
	sfz_atk				=	70
	sus_atk				=	70
	mleg_atk			= 	70
	--Release Compensation Constants
	lgt_release			=	80
	
	--Keyswitch Offset Constants
	keysw_lead      	=   10
	
	--Short Note Constants
	short_uniform_len 	= 	150
	trs_stctsm			=	MsSmps(40)
	trs_stac			= 	trs_stctsm*2
	trs_sfz 			=	trs_stctsm*3
	trs_short 			= 	trs_stctsm*4
	
	--Script Constants
	PlayheadPosRange	= (LUAINTMAX	- 44100) --generally a block won't be larger than 44100 smps
	
	--Structure Definitions
	Note = {
		Type,
		NoteOnTime,
		OnTimeTransformed,
		NoteEndTime,
		EndTimeTransformed,
		LegatoLive,
		AssociatedTimer,
		NoteVelo,
		Prev,
		ToleranceState,
		ToleranceTimer,
		NoteNum
	}
	
	Timer = {
		AssociatedNote,
		TimerID,
		ValuePassed,
		Type
	}
	
	--Enum Tables
	NoteType = {
		pending 		= 1,
		short			= 2,
		long = {
			pending		= 3,
			sustain		= 4,
			legato = {
				pending	= 5,
				start	= 6,
				follow	= 7
			}
		}
	}
	TimerType = {
		length 			= 1,
		tolerance 		= 2
	}
	
	--Script Global Variables
	NotesList 			= {}
	TimersList			= {}
	LastNote			= -1
	PlayheadPos			= 0

	--Script Control Definition


	ProcessingTypes = {
		fixed		= 1, --fixed compensation at start
		legato		= 2, --use legato type
	}

	--Always make legato variant = primary + 1
	--Always make legato variant ODD, primary EVEN
	--For reason, see UpdateSelectedTech()
	--[[

	Tech = {
		int 		techId,
		string 		techName,
		int			processingType,
		bool		isVarilengthShort,

		string			primaryVariant,		--Only fill both of these two fields if the tech has OR is a legato variant
		string			legatoVariant
	}
	]]

	iTechId 		= 1
	iTechName		= 2
	iTechProcType 	= 3
	iTechIsVarShrt	= 4
	iTechPrimary	= 5
	iTechLegato		= 6
	Techs = {
		sustain 	= { 2		,'sustain'		,ProcessingTypes.fixed		,false	,'sustain'	,'legato'	},
		legato		= { 3		,'legato'		,ProcessingTypes.legato		,false	,'sustain'	,'legato'	},

		shortSfz	= { 4		,'shortSfz'		,ProcessingTypes.fixed		,true	},
		shortStac	= { 5		,'shortStac'	,ProcessingTypes.fixed		,true	},
		shortStctsm = { 6		,'shortStctsm'	,ProcessingTypes.fixed		,true	},
		shortSpicc	= { 7		,'shortSpicc'	,ProcessingTypes.fixed		,true	},

		marcato 	= { 8		,'marcato'		,ProcessingTypes.fixed		,false	,'marcato'	,'mlegato'	},
		mlegato		= { 9		,'mlegato'		,ProcessingTypes.legato		,false	,'marcato'	,'mlegato'	},

		tremsus		= { 10		,'tremsus'		,ProcessingTypes.fixed		,false	,'tremsus'	,'tremlgt'	},
		tremlgt		= { 11		,'tremlgt'		,ProcessingTypes.legato		,false	,'tremsus'	,'tremlgt'	},

		mutesus		= { 12		,'mutesus'		,ProcessingTypes.fixed		,false	,'mutesus'	,'muteleg'	},
		muteleg		= { 13		,'muteleg'		,ProcessingTypes.legato		,false	,'mutesus'	,'muteleg'	},
		muteshrt	= { 14		,'muteshrt'		,ProcessingTypes.fixed		,false	},

		trillsus	= { 16		,'trillsus'		,ProcessingTypes.fixed		,false	,'trillsus' ,'trilllgt' },
		trilllgt	= { 17		,'trilllgt'		,ProcessingTypes.legato		,false	,'trillsus' ,'trilllgt' },

		synctrem	= { 18		,'synctrem'		,ProcessingTypes.fixed		,false	},

		rips		= { 19		,'rips'			,ProcessingTypes.fixed		,false	}
	}
	CurrentSelectedTech 	= 'legato'

	EnableSwitchCtrl 		= 4
	LegatoSwitchCtrl		= 5
	MarcatoSwitchCtrl		= 6
	ForceLengthTypeCtrl		= 9
	LegatoModeOn 			= true
	TransformEnabled		= true
	MarcatoModeOn			= false
	ForceLength				= 0 		-- 1 = Force Long
	
	--Triage Functions
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
	
	function chk_marcato_mode()
		return MarcatoModeOn
	end
	
	function true_func() return true end
	
	--Keyswitch Functions
	function switch_to_spicc(eventoffset) 
		PostKeyEvt('f0',127,eventoffset)
		PostControllerEvt(1,1,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_stctsm(eventoffset) 
		PostKeyEvt('f0',127,eventoffset)
		PostControllerEvt(1,40,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_stac(eventoffset)
		PostKeyEvt('f0',127,eventoffset)
		PostControllerEvt(1,80,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_sfz(eventoffset)
		PostKeyEvt('f0',127,eventoffset)
		PostControllerEvt(1,127,eventoffset+MsSmps(keysw_lead/2))
	end
	function switch_to_lgt(eventoffset)
		PostKeyEvt('c0',127,eventoffset)
		PostKeyEvt('bb0',127,eventoffset+MsSmps(1))
	end
	function switch_to_sustain(eventoffset)
		PostKeyEvt('c0',127,eventoffset)
		PostKeyEvt('bb0',1,eventoffset+MsSmps(1))
	end
	function switch_to_mlgt(eventoffset)
		PostKeyEvt('f#0',127,eventoffset)
		PostKeyEvt('bb0',127,eventoffset+MsSmps(1))
	end
	function switch_to_marcato(eventoffset)
		PostKeyEvt('f#0',127,eventoffset)
		PostKeyEvt('bb0',1,eventoffset+MsSmps(1))
	end
	
	--Triage Tables (sorted by priority)
	
	function Triage(TriageTable, ... )		-- ... = arguments supplied to triage functions
		for i,item in pairs(TriageTable) do	
			if item[1](...) then			-- if the condition is fulfilled, return the item containing corresponding information
				return item
			end
		end
	end
	
	shortNoteTriageTable = {
		[1] = { chk_len_spicc 	, spicc_atk	, switch_to_spicc	},
		[2]	= { chk_len_stac 	, stac_atk	, switch_to_stac	},
		[3] = { chk_len_stctsm	, stctsm_atk, switch_to_stctsm	},
		[4] = { chk_len_sfz		, sfz_atk	, switch_to_sfz		}
	}
	
	legatoSpeedTriageTable = {
		[1] = { chk_marcato_mode 	, lgt_mleg 		, mlgt_overlap	},
		[2] = { chk_vel_lgt_fast 	, lgt_fast 		, lgt_overlap	},
		[3] = { chk_vel_lgt_medium 	, lgt_medium 	, lgt_overlap	}
	}
	
	legatoAttackTriageTable = {
		[1] = { chk_marcato_mode 	, mleg_atk		, switch_to_mlgt },
		[2] = { true_func	 		, lgt_atkfast	, switch_to_lgt }	--default option
	}
	
	sustainTriageTable = {
		[1] = { chk_marcato_mode	, mleg_atk	, switch_to_marcato },
		[2] = { true_func			, sus_atk	, switch_to_sustain	}	--default option
	}

	--Helper Functions
	function RemoveTimer(tid)
		TimersList[tid] = nil
	end
	function RemoveNote(nid)
		NotesList[nid] = nil
	end
	function NoteExist(nid)
		if NotesList[nid] ~= nil then
			return true
		else
			return false
		end
	end
	function TimerExist(tid)
		if TimersList[tid] ~= nil then
			return true
		else
			return false
		end
	end
	function GetNoteTimerId(nid)
		if NoteExist(nid) then
			return NotesList[nid].AssociatedTimer
		else
			return 0
		end
	end
	function GetTimerNoteId(tid)
		if TimerExist(tid) then
			return TimersList[tid].AssociatedNote
		else
			return 0
		end
	end
	function GetNoteTimer(nid)
		return TimersList[GetNoteTimerId(nid)]
	end
	function GetTimerNote(tid)
		return NotesList[GetTimerNoteId(tid)]
	end
	function NoteTimerExist(nid)
		return TimerExist(GetNoteTimerId(nid))
	end
	function TimerNoteExist(tid)
		return NoteExist(GetTimerNoteId(tid))
	end

	--Debug Helpers
	NoteTypeNames = {
		"PENDING      ",
		"SHORT        ",
		"LONG    ?    ",
		"SUSTAIN      ",
		"LEGATO  ?    ",
		"LEGSTART     ",
		"LEGFOLLOW    ",
	}
	function NoteInfoStr(nid)
		return GetNoteName(NotesList[nid].NoteNum,8).."    id= "..StrFormatLen(nid,8).."type= "..NoteTypeNames[NotesList[nid].Type]
	end

	--Alt process type utilities
	cc1_trs_sfz 		= 96
	cc1_trs_stac		= 64
	cc1_trs_stctsm		= 32
	cc1_trs_spicc		= 0

	--TODO
	function chk_cc1_sfz(cc1val)
		return cc1val >= cc1_trs_sfz
	end

	function chk_cc1_stac(cc1val)
		return cc1_trs_stac <= cc1val and cc1val < cc1_trs_sfz
	end

	function chk_cc1_stctsm(cc1val)
		return cc1_trs_stctsm <= cc1val and cc1val < cc1_trs_stac
	end	

	function chk_cc1_spicc(cc1val)
		return cc1val < cc1_trs_stctsm
	end

	shortNoteCC1TriageTable = {
		[1] = { chk_cc1_spicc 	, 'shortSpicc'	},
		[2]	= { chk_cc1_stac 	, 'shortStac'	},
		[3] = { chk_cc1_stctsm	, 'shortStctsm'	},
		[4] = { chk_cc1_sfz		, 'shortSfz'	}
	}

	primaryKsws = 
	{
		[NoteNum('c0')] 	= 'sustain',
		[NoteNum('c#0')]	= 'mutesus',
		[NoteNum('d0')]		= 'synctrem',
		[NoteNum('d#0')]	= 'trillsus',
		[NoteNum('e0')]		= 'tremsus',
		[NoteNum('f0')]		= 'shortSfz',
		[NoteNum('f#0')]	= 'marcato',
		[NoteNum('g0')]		= 'rips'
	}

	secondaryKsws =
	{
		[NoteNum('bb0')] 	= 'legato_control'
	}

	lastUsedShortType = Techs.shortSfz[iTechName]

end
--===========================================================================================

function init_script()
	init_utils()
	CurrentSelectedTech = 0
	DebugMessage("script initialized, samplerate = ", samplerate)
end

function ProcessShortNote(nid) 
	NotesList[nid].Type = NoteType.short
	local noteLen = PlayheadPos-NotesList[nid].NoteOnTime
	
	--Select correction and keyswitch
	local triageItem = Triage(shortNoteTriageTable, noteLen)
	local shortCorrection = triageItem[2]
	local keyswFunc = triageItem[3]

	keyswFunc(NotesList[nid].NoteOnTime - PlayheadPos - MsSmps(shortCorrection)-MsSmps(keysw_lead))
	PostMsg(NOTEON,NotesList[nid].NoteNum,NotesList[nid].NoteVelo,NotesList[nid].NoteOnTime - PlayheadPos- MsSmps(shortCorrection))
	PostMsg(NOTEOFF,NotesList[nid].NoteNum,64,NotesList[nid].NoteOnTime - PlayheadPos - MsSmps(shortCorrection) + MsSmps(short_uniform_len))
	
	DebugMessage(StrFormatLen("SHORT ON",15)..NoteInfoStr(nid))
end
function SustainNoteOff(nid)
	--Process sustain note off
	PostMsg(NOTEOFF,NotesList[nid].NoteNum,value,0)

	DebugMessage(StrFormatLen("NOTE OFF",15)..NoteInfoStr(nid))
end
function LegatoNoteEnterToleranceState(nid)
	NotesList[nid].ToleranceState = true
	local tid = NewTimer(trs_lgt_continue+trs_short,nid)
	NotesList[nid].AssociatedTimer = tid
	TimersList[tid] = {
		AssociatedNote = nid,
		TimerID = tid,
		ValuePassed = nid,
		Type = TimerType.tolerance
	}

	DebugMessage(StrFormatLen("T-STATE ON",15)..NoteInfoStr(nid))
end
function isLegatoNoteFollow(nid)	--return: bool isFollow {, int PrevID}
	if 	NotesList[NotesList[nid].Prev] == nil or 									--No previous note.
		NotesList[NotesList[nid].Prev].NoteNum == NotesList[nid].NoteNum then 		--Same note repetition.
		return false
	elseif NotesList[NotesList[nid].Prev].LegatoLive then							--Has previous note, and prev note is live
		return true, NotesList[nid].Prev
	else
		return false
	end
end
function LegatoFollowNoteOn(nid,pid)	--return: bool isInToleranceState {, int ToleranceTimerID}
	--Select legato transition length
	local triageItem = Triage(legatoSpeedTriageTable,NotesList[nid].NoteVelo)
	local compensation = triageItem[2]
	local overlap = triageItem[3]

	NotesList[nid].Type = NoteType.long.legato.follow
	NotesList[nid].LegatoLive = true
	--Send note on
	PostMsg(NOTEON,NotesList[nid].NoteNum,NotesList[nid].NoteVelo,(NotesList[nid].NoteOnTime-PlayheadPos) - MsSmps(compensation))
	--Send previous note off after a short period of time
	PostMsg(NOTEOFF,NotesList[pid].NoteNum,64,(NotesList[nid].NoteOnTime -PlayheadPos)- MsSmps(compensation) +MsSmps(overlap))

	DebugMessage(StrFormatLen("NOTE ON",15)..NoteInfoStr(nid))
	DebugMessage(StrFormatLen("NOTE OFF FLW",15)..NoteInfoStr(pid))

	--If the previous note is in tolerance state, return extra info
	if NotesList[pid].ToleranceState == true then
		return true, NotesList[pid].AssociatedTimer
	end
	return false
end
function LegatoStartNoteOn(nid)
	--Select attack compensation and keyswitch
	local triageItem = Triage(legatoAttackTriageTable)
	local compensation = triageItem[2]
	local keySwFunc = triageItem[3]
	
	--Process note
	NotesList[nid].Type = NoteType.long.legato.start
	NotesList[nid].LegatoLive = true
	keySwFunc((NotesList[nid].NoteOnTime -PlayheadPos)- MsSmps(compensation) - MsSmps(keysw_lead))
	PostMsg(NOTEON,NotesList[nid].NoteNum,NotesList[nid].NoteVelo,(NotesList[nid].NoteOnTime -PlayheadPos)- MsSmps(compensation))

	DebugMessage(StrFormatLen("NOTE ON",15)..NoteInfoStr(nid))
end

function SustainNoteOn(nid)
	NotesList[nid].Type = NoteType.long.sustain
					
	--Select technique and compensation
	local triageItem = Triage(sustainTriageTable)
	local compensation = triageItem[2]
	local keySwFunc = triageItem[3]
	
	--Process sustain note
	keySwFunc((NotesList[nid].NoteOnTime -PlayheadPos)- MsSmps(compensation) - MsSmps(keysw_lead))
	PostMsg(NOTEON,NotesList[nid].NoteNum,NotesList[nid].NoteVelo,(NotesList[nid].NoteOnTime-PlayheadPos) - MsSmps(compensation))

	DebugMessage(StrFormatLen("NOTE ON",15)..NoteInfoStr(nid))
end
function LegatoFinalNoteOff(nid)
	NotesList[nid].LegatoLive = false;
	NotesList[nid].ToleranceState = false;
	PostMsg(NOTEOFF,NotesList[nid].NoteNum,64,(NotesList[nid].NoteEndTime-PlayheadPos)-MsSmps(lgt_release))

	DebugMessage(StrFormatLen("LGT FINAL",15)..NoteInfoStr(nid))
end
function NoteEnterLengthPendingState(nid)	--return : int timerID
	local tid = NewTimer(trs_short,nid)
	TimersList[tid] ={
		AssociatedNote = nid,
		TimerID = NotesList[nid].AssociatedTimer,
		ValuePassed = nid,
		Type = TimerType.length
	}
	NotesList[nid].AssociatedTimer = tid
	return tid 
end


--TODO
function UpdateSelectedTech(isNote, control, val) -- return true if the note should be bypassed
	local updatedTech = CurrentSelectedTech
	local isKswNote = false
	if isNote then
		local newTechName = primaryKsws[control]
		if newTechName ~= nil then --newTechName exist in primaryKsws, means the note is a primary keyswitch
			isKswNote = true
			if newTechName == 'shortSfz' then
				updatedTech = lastUsedShortType
			elseif Techs[newTechName][iTechLegato] == nil then
				updatedTech = Techs[newTechName][iTechName]
			else
				if LegatoModeOn then
					updatedTech = Techs[newTechName][iTechLegato]
				else
					updatedTech = Techs[newTechName][iTechPrimary]
				end
				
				--specially...
				if newTechName == 'mutesus' and val<64 then
					updatedTech = Techs.muteshrt[iTechName]
				end
			end
		else --newTechName is empty, this is a secondary keyswitch
			local secondaryType = secondaryKsws[control]
			if secondaryType == 'legato_control' then
				isKswNote = true
				local newLgtModeOn = BoolController(val)
				if newLgtModeOn ~= LegatoModeOn then --skip if nothing changed
					LegatoModeOn = newLgtModeOn
					if Techs[CurrentSelectedTech][iTechLegato] ~= nil then
						if LegatoModeOn then
							updatedTech = Techs[CurrentSelectedTech][iTechLegato]
						else
							updatedTech = Techs[CurrentSelectedTech][iTechPrimary]
						end
					end
				end
			end
		end

	else
		if control == 1 and techIsVarilengthShort[CurrentSelectedTech] then
			local triageItem = Triage(shortNoteCC1TriageTable,val)
			updatedTech = triageItem[2]
			lastUsedShortType = triageItem[2]
		end
	end

	if updatedTech ~= CurrentSelectedTech then
		CurrentSelectedTech = updatedTech
		DebugMessage("Technique Updated : ", CurrentSelectedTech)
	end

	if isKswNote then
		return true
	end
end

--TODO
function message_income(msgtype,control,value,assignid)

	--always update technique state, and bypass all keyswitch related notes
	if msgtype == CONTROLLER then
		UpdateSelectedTech(false,control,value)
	elseif msgtype == NOTEON then
		if UpdateSelectedTech(true,control,value) then
			PostMsg(msgtype,control,value,0)
			return
		end
	elseif msgtype == NOTEOFF then
		if primaryKsws[control] or secondaryKsws[control] then
			PostMsg(msgtype,control,value,0)
			return
		end
	end

	if msgtype == CONTROLLER --[[and control is one of the switches]] then
		--Process switches
		local isSwitches = false
		if control == EnableSwitchCtrl then
			isSwitches = true
			TransformEnabled = BoolController(value)
			--clean up when bypassed
			if not TransformEnabled then
				NotesList = {}
				TimersList = {}
				LastNote = -1
			end
		end
		
		if isSwitches then
			return
		end
		
		PostMsg(msgtype,control,value,0)
	end --return after switch is processed

	if not TransformEnabled then --Bypassed, forward NoteOn NoteOff and Controllers
		if msgtype == CONTROLLER or msgtype == NOTEON or msgtype == NOTEOFF then
			PostMsg(msgtype,control,value,0)
		end
		return
	end --return after forwarding message

	if msgtype == CONTROLLER then
		--Bypass all controllers other than switches
		PostMsg(msgtype,control,value,0)
	elseif msgtype == NOTEON then
		local nid = assignid
		NotesList[nid] = {
			Type = NoteType.pending,
			NoteOnTime = PlayheadPos,
			NoteVelo = value,
			Prev = LastNote,
			NoteNum = control
		}
		NoteEnterLengthPendingState(nid)
		LastNote = nid
	elseif msgtype == TIMER then
		local tid = control

		if TimerExist(control) then
			local nid = TimersList[tid].AssociatedNote

			--Long note process
			if TimersList[tid].Type == TimerType.length then
				NotesList[nid].Type = NoteType.long.pending

				--Legato
				if LegatoModeOn then
					--Determine if the legato note is START or FOLLOW
					local isFollow, pid = isLegatoNoteFollow(nid)

					--Process Follow note
					if isFollow then
						--Process follow note
						local isInToleranceState, ToleranceTimer = LegatoFollowNoteOn(nid, pid)
						--Conditional cleanup
						if isInToleranceState then
							RemoveTimer(ToleranceTimer)
						end
						--Cleanup
						RemoveNote(pid)
					--Process Start Note
					else
						LegatoStartNoteOn(nid)
					end
				--Sustain
				else
					SustainNoteOn(nid)
				end
			--ExitLegatoState process
			elseif TimersList[tid].Type == TimerType.tolerance then
				if TimerNoteExist(tid) then
					LegatoFinalNoteOff(nid)
				end
				RemoveNote(nid)
			end

			RemoveTimer(control)
		end
	elseif msgtype == NOTEOFF then
		if not NoteExist(assignid) then
		else
			--Data prep
			NotesList[assignid].NoteEndTime = PlayheadPos
			local nid = assignid
			local tid = GetNoteTimerId(assignid)
		
			if NotesList[assignid].Type == NoteType.pending then
				--Process short note
				ProcessShortNote(nid)
				RemoveNote(nid)
				RemoveTimer(tid)

			elseif NotesList[assignid].Type == NoteType.long.sustain then
				--Process sustain note off
				SustainNoteOff(nid)
				RemoveNote(nid)
				RemoveTimer(tid)

			elseif NotesList[assignid].Type == NoteType.long.legato.start or NotesList[assignid].Type == NoteType.long.legato.follow then
				--Process legato note off
				LegatoNoteEnterToleranceState(nid)
				--RemoveNote(nid)  						leave it there for future reading
				--PostMsg(NOTEOFF,control,value,0)		don't send noteoff, wait for next possible followers
			end
		end
	end
end

function advance_time(samples)
	PlayheadPos = PlayheadPos + samples
end
