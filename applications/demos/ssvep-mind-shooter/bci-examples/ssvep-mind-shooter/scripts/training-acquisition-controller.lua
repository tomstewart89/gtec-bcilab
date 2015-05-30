sequence = {}
number_of_cycles = 0

stimulation_duration = 7
break_duration = 2
flickering_delay = 3.5

stimulationLabels = {
	0x00008100,
	0x00008101,
	0x00008102,
	0x00008103,
}

function initialize(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")

	-- load the goal sequence
	s_sequence = box:get_setting(2)

	for target in s_sequence:gmatch("%d+") do
		table.insert(sequence, target)
		number_of_cycles = number_of_cycles + 1
	end

	stimulation_duration = box:get_setting(3)
	break_duration = box:get_setting(4)
	flickering_delay = box:get_setting(5)

	box:log("Info", string.format("Number of goals in sequence: [%d]", number_of_cycles))

	cfg_file = io.open(box:get_config("${Player_ScenarioDirectory}/configuration/stimulation-based-epoching.cfg"), "w")

	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write("	<SettingValue>", stimulation_duration, "</SettingValue>\n")
	cfg_file:write("	<SettingValue>", flickering_delay, "</SettingValue>\n")
	cfg_file:write("	<SettingValue>OVTK_StimulationId_Target</SettingValue>\n")
	cfg_file:write("</OpenViBE-SettingsOverride>\n")

	cfg_file:close()
end

function uninitialize(box)
end

function process(box)

	while box:keep_processing() and box:get_stimulation_count(1) == 0 do
		box:sleep()
	end

	current_time = box:get_current_time() + 1

	box:send_stimulation(1, OVTK_StimulationId_ExperimentStart, current_time, 0)

	current_time = current_time + 2

	for i,target in ipairs(sequence) do
		box:log("Info", string.format("Goal no %d is %d at %d", i, target, current_time))
		-- mark goal
		box:send_stimulation(2, OVTK_StimulationId_LabelStart + target, current_time, 0)
		-- wait for Flickering_delay seconds
		current_time = current_time + flickering_delay
		-- start flickering
		box:send_stimulation(1, OVTK_StimulationId_VisualStimulationStart, current_time, 0)
		-- wait for Stimulation_duration seconds
		current_time = current_time + stimulation_duration
		-- unmark goal and stop flickering
		box:send_stimulation(1, OVTK_StimulationId_VisualStimulationStop, current_time, 0)
		-- wait for Break_duration seconds
		current_time = current_time + break_duration
	end

	box:send_stimulation(1, OVTK_StimulationId_ExperimentStop, current_time, 0)

	box:sleep()
end
