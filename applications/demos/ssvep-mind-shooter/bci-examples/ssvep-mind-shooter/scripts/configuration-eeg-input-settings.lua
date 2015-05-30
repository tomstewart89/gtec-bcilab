channels = nil
selection_method = nil
match_method = nil

training_datafile = nil
testing_datafile = nil

function initialize(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")
	channels = box:get_setting(2)
	selection_method = box:get_setting(3)
	match_method = box:get_setting(4)
	training_datafile = box:get_setting(5)
	testing_datafile = box:get_setting(6)


end

function uninitialize(box)
end

function process(box)

	while box:keep_processing() and box:get_stimulation_count(1) == 0 do
		box:sleep()
	end

	-- create material

	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/channel-selector.cfg'"))

	cfg_file = io.open(box:get_config("${Player_ScenarioDirectory}/configuration/channel-selector.cfg"), "w")

	if cfg_file == nil then
		box:log("Error", "Could not open config file for writing")
	end

	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write(string.format("<SettingValue>%s</SettingValue>\n", channels))
	cfg_file:write(string.format("<SettingValue>%s</SettingValue>\n", selection_method))
	cfg_file:write(string.format("<SettingValue>%s</SettingValue>\n", match_method))
	cfg_file:write("</OpenViBE-SettingsOverride>\n")

	cfg_file:close()

	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/file-reader-training.cfg'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/configuration/file-reader-training.cfg"), "w"))

	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write(string.format("<SettingValue>%s</SettingValue>\n", training_datafile))
	-- cfg_file:write(string.format("<SettingValue>32</SettingValue>\n"))
	cfg_file:write("</OpenViBE-SettingsOverride>\n")

	cfg_file:close()

	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/file-reader-testing.cfg'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/configuration/file-reader-testing.cfg"), "w"))

	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write(string.format("<SettingValue>%s</SettingValue>\n", testing_datafile))
	-- cfg_file:write(string.format("<SettingValue>32</SettingValue>\n"))
	cfg_file:write("</OpenViBE-SettingsOverride>\n")

	cfg_file:close()




	-- notify the scenario that the configuration process is complete
	box:send_stimulation(1, OVTK_StimulationId_TrainCompleted, box:get_current_time() + 0.2, 0)

end
