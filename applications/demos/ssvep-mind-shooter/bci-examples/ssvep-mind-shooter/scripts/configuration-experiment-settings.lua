stimulation_frequencies = {}
frequency_count = 0

target_light_color = {}
target_dark_color = {}
training_target_size = {}
training_targets_positions = {}

processing_epoch_duration = nil
processing_epoch_interval = nil
processing_frequency_tolerance = nil
csp_filter_order = nil

function initialize(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")
	for value in box:get_setting(2):gmatch("%d+") do
		table.insert(target_light_color, value)
	end

	for value in box:get_setting(3):gmatch("%d+") do
		table.insert(target_dark_color, value)
	end

	for value in box:get_setting(4):gmatch("%d+[.]?%d*") do
		table.insert(stimulation_frequencies, value)
		frequency_count = frequency_count + 1
	end

	processing_epoch_duration = box:get_setting(5)
	processing_epoch_interval = box:get_setting(6)
	processing_frequency_tolerance = box:get_setting(7)
	csp_filter_order = box:get_setting(8)
	s_focus_point = box:get_setting(9)


end

function uninitialize(box)
end

function process(box)

	while box:keep_processing() and box:get_stimulation_count(1) == 0 do
		box:sleep()
	end

	-- create material

	box:log("Info", box:get_config("Creating materials in '${Player_ScenarioDirectory}/appconf/materials/flickering.material'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/appconf/materials/flickering.material"), "w"))

	cfg_file:write("material target_bright\n")
	cfg_file:write("{\n")
	cfg_file:write("	technique\n")
	cfg_file:write("	{\n")
	cfg_file:write("		pass\n")
	cfg_file:write("		{\n")
	cfg_file:write("			ambient 0 0 0 0\n")
	cfg_file:write("			diffuse 0 0 0 0\n")
	cfg_file:write("			specular 0 0 0 0\n")
	cfg_file:write("			emissive ", target_light_color[1] / 100, " ", target_light_color[2] / 100, " ", target_light_color[3] / 100, "\n")
	cfg_file:write("		}\n")
	cfg_file:write("	}\n")
	cfg_file:write("}\n")
	cfg_file:write("material target_dark\n")
	cfg_file:write("{\n")
	cfg_file:write("	technique\n")
	cfg_file:write("	{\n")
	cfg_file:write("		pass\n")
	cfg_file:write("		{\n")
	cfg_file:write("			ambient 0 0 0 0\n")
	cfg_file:write("			diffuse 0 0 0 0\n")
	cfg_file:write("			specular 0 0 0 0\n")
	cfg_file:write("			emissive ", target_dark_color[1] / 100, " ", target_dark_color[2] / 100, " ", target_dark_color[3] / 100, "\n")
	cfg_file:write("		}\n")
	cfg_file:write("	}\n")
	cfg_file:write("}\n")
	cfg_file:close()

	-- prepare the datafolder
	
	foldername = box:get_config("${Player_ScenarioDirectory}/signals/userdata-[$core{date}-$core{time}]")
	os.execute(string.format("mkdir \"%s\"", foldername))

	box:log("Info", box:get_config("Writing additional configuration to '${Player_ScenarioDirectory}/appconf/application-configuration.conf'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/appconf/application-configuration.conf"), "a"))

	for i=1,frequency_count do
		cfg_file:write("SSVEP_Frequency_", i, " = ", string.format("%g", stimulation_frequencies[i]), "\n")
	end

	cfg_file:write(string.format("SSVEP_TargetLightColourRed = %f\n", target_light_color[1] / 100))
	cfg_file:write(string.format("SSVEP_TargetLightColourGreen = %f\n", target_light_color[2] / 100))
	cfg_file:write(string.format("SSVEP_TargetLightColourBlue = %f\n", target_light_color[3] / 100))
	cfg_file:write(string.format("SSVEP_TargetDarkColourRed = %f\n", target_dark_color[1] / 100))
	cfg_file:write(string.format("SSVEP_TargetDarkColourGreen = %f\n", target_dark_color[2] / 100))
	cfg_file:write(string.format("SSVEP_TargetDarkColourBlue = %f\n", target_dark_color[3] / 100))

	cfg_file:write("SSVEP_UserDataFolder = ", foldername, "\n")
	cfg_file:write("SSVEP_FocusPoint = ", s_focus_point, "\n")


	cfg_file:close()

	-- create configuration files for temporal filters

	for i=1,frequency_count do
		box:log("Info", string.format(box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/temporal-filter-freq-%db.cfg'"), i))

		cfg_file = assert(io.open(string.format(box:get_config("${Player_ScenarioDirectory}/configuration/temporal-filter-freq-%db.cfg"), i), "w"))

		cfg_file:write("<OpenViBE-SettingsOverride>\n")
		cfg_file:write("<SettingValue>Butterworth</SettingValue>\n")
		cfg_file:write("<SettingValue>Band pass</SettingValue>\n")
		cfg_file:write("<SettingValue>4</SettingValue>\n")
		cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", stimulation_frequencies[i] - processing_frequency_tolerance))
		cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", stimulation_frequencies[i] + processing_frequency_tolerance))
		cfg_file:write("<SettingValue>0.500000</SettingValue>\n")
		cfg_file:write("</OpenViBE-SettingsOverride>\n")

		cfg_file:close()

		box:log("Info", string.format(box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/temporal-filter-freq-%dh1.cfg'"), i))

		cfg_file = assert(io.open(string.format(box:get_config("${Player_ScenarioDirectory}/configuration/temporal-filter-freq-%dh1.cfg"), i), "w"))

		cfg_file:write("<OpenViBE-SettingsOverride>\n")
		cfg_file:write("<SettingValue>Butterworth</SettingValue>\n")
		cfg_file:write("<SettingValue>Band pass</SettingValue>\n")
		cfg_file:write("<SettingValue>4</SettingValue>\n")
		cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", stimulation_frequencies[i] * 2 - processing_frequency_tolerance))
		cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", stimulation_frequencies[i] * 2 + processing_frequency_tolerance))
		cfg_file:write("<SettingValue>0.500000</SettingValue>\n")
		cfg_file:write("</OpenViBE-SettingsOverride>\n")

		cfg_file:close()

		box:log("Info", string.format(box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/csp-spatial-filter-trainer-%db.cfg'"), i))

		cfg_file = assert(io.open(string.format(box:get_config("${Player_ScenarioDirectory}/configuration/csp-spatial-filter-trainer-%db.cfg"), i), "w"))
		cfg_file:write("<OpenViBE-SettingsOverride>\n")
		cfg_file:write("	<SettingValue>OVTK_StimulationId_ExperimentStop</SettingValue>\n")
		cfg_file:write(string.format(box:get_config("	<SettingValue>${Player_ScenarioDirectory}/classifiers/csp-%db</SettingValue>\n"), i))
		cfg_file:write(string.format("	<SettingValue>%d</SettingValue>\n", csp_filter_order))
		cfg_file:write("</OpenViBE-SettingsOverride>\n")
		cfg_file:close()

		box:log("Info", string.format(box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/csp-spatial-filter-trainer-%dh1.cfg'"), i))

		cfg_file = assert(io.open(string.format(box:get_config("${Player_ScenarioDirectory}/configuration/csp-spatial-filter-trainer-%dh1.cfg"), i), "w"))
		cfg_file:write("<OpenViBE-SettingsOverride>\n")
		cfg_file:write("	<SettingValue>OVTK_StimulationId_ExperimentStop</SettingValue>\n")
		cfg_file:write(string.format(box:get_config("	<SettingValue>${Player_ScenarioDirectory}/classifiers/csp-%dh1</SettingValue>\n"), i))
		cfg_file:write(string.format("	<SettingValue>%d</SettingValue>\n", csp_filter_order))
		cfg_file:write("</OpenViBE-SettingsOverride>\n")
		cfg_file:close()

	end

	-- create configuration file for time based epoching
	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/time-based-epoching.cfg'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/configuration/time-based-epoching.cfg"), "w"))
	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", processing_epoch_duration))
	cfg_file:write(string.format("<SettingValue>%g</SettingValue>\n", processing_epoch_interval))
	cfg_file:write("</OpenViBE-SettingsOverride>\n")
	cfg_file:close()

	-- create the file writer configuration
	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/file-writer.cfg'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/configuration/file-writer.cfg"), "w"))
	cfg_file:write("<OpenViBE-SettingsOverride>\n")
	cfg_file:write(string.format("<SettingValue>%s/eeg-data-[$core{date}-$core{time}].ov</SettingValue>\n", foldername))
	cfg_file:write("<SettingValue>false</SettingValue>\n")
	cfg_file:write("</OpenViBE-SettingsOverride>\n")
	cfg_file:close()
	
	box:log("Info", box:get_config("Writing file '${Player_ScenarioDirectory}/configuration/csp-filter.cfg'"))

	-- notify the scenario that the configuration process is complete
	box:send_stimulation(1, OVTK_StimulationId_TrainCompleted, box:get_current_time() + 0.2, 0)

end
