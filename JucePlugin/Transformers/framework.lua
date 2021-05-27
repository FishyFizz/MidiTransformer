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

function init_script()
	--PLACE CODE HERE
end

function message_income(msgtype,control,value,assignid)
	--PLACE CODE HERE
end

function advance_time(samples)
	--PLACE CODE HERE
end
