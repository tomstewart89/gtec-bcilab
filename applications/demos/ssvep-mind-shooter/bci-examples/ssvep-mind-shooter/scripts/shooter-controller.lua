sequence = {}
flickeringDelay = nil

function initialize(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")
	s_sequence_types = box:get_setting(2)
	s_sequence_positions = box:get_setting(3)
	s_target_lockdown = box:get_setting(4)
	s_pilot_assist = box:get_setting(5)
	s_feedback = box:get_setting(6)
	s_onebyone = box:get_setting(7)

	box:log("Info", string.format("Target Types Sequence : [%s]\n", s_sequence_types))

	for target in s_sequence_positions:gmatch("%d+") do
		table.insert(sequence, target)
	end
	
	-- create the configuration file for the shooter application
	cfg_file = io.open(box:get_config("${Player_ScenarioDirectory}/appconf/impact-configuration.conf"), "w")

	cfg_file:write("SSVEP_EnemyOrder = ", s_sequence_types, "\n")
	cfg_file:write("SSVEP_TargetLockdown = ", s_target_lockdown, "\n")
	cfg_file:write("SSVEP_PilotAssist = ", s_pilot_assist, "\n")
	cfg_file:write("SSVEP_Feedback = ", s_feedback, "\n")
	cfg_file:write("SSVEP_OneByOne = ", s_onebyone, "\n")
	cfg_file:close()
end

function uninitialize(box)
end

function process(box)

	-- while targets are available, send a stimulation with new target
	-- position for each request

	while box:keep_processing() do
		if box:get_stimulation_count(1) > 0 then

			identifier, date, duration = box:get_stimulation(1, 1)

			if identifier == OVTK_StimulationId_Target then
				current_time = box:get_current_time() + 1

				-- if no targets are available, quit the game
				if # sequence == 0 then
					box:send_stimulation(1, OVTK_StimulationId_ExperimentStop, current_time, 0)
					break
				end

				next_target = sequence[1]
				table.remove(sequence, 1)

				box:send_stimulation(1, OVTK_StimulationId_Label_00 + next_target , current_time, 0)
			end

			box:remove_stimulation(1, 1)
		end

		box:sleep()
	end

end

