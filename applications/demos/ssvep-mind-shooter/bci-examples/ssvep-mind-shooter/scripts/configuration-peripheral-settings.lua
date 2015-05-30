vrpn_host = nil
vrpn_port = nil
screen_refresh_rate = nil
window_width = nil
window_height = nil
fullscreen = nil

function initialize(box)
	screen_refresh_rate = box:get_setting(2)
	window_width = box:get_setting(3)
	window_height = box:get_setting(4)
	fullscreen = box:get_setting(5)
end

function uninitialize(box)
end

function process(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")

	box:log("Info", box:get_config("Generating '${Player_ScenarioDirectory}/appconf/application-configuration.conf'"))

	cfg_file = assert(io.open(box:get_config("${Player_ScenarioDirectory}/appconf/application-configuration.conf"), "w"))

	cfg_file:write("# This file was automatically generated!\n\n")
	cfg_file:write("# If you want to change the SSVEP configuration\n")
	cfg_file:write("# please use the provided ssvep-configuration scenario.\n")

	cfg_file:write("SSVEP_ScreenRefreshRate = ", screen_refresh_rate, "\n")
	cfg_file:write("SSVEP_VRPNServerPort = 1337\n")
	cfg_file:write("SSVEP_VRPNHost = localhost\n")

--	cfg_file:close()

--	box:log("Info", box:get_config("Generating '${Player_ScenarioDirectory}/appconf/ogre-configuration.conf'"))

--	cfg_file = io.open(box:get_config("${Player_ScenarioDirectory}/appconf/ogre-configuration.conf"), "w")

--	cfg_file:write("# This file was automatically generated!\n\n")
--	cfg_file:write("# If you want to change the SSVEP configuration\n")
--	cfg_file:write("# please use the provided ssvep-configuration scenario.\n")

	cfg_file:write("SSVEP_Ogre_LogToConsole = False\n")

	cfg_file:write("SSVEP_Ogre_ScreenWidth = ", window_width, "\n")
	cfg_file:write("SSVEP_Ogre_ScreenHeight = ", window_height, "\n")
	cfg_file:write("SSVEP_Ogre_FullScreen = ", fullscreen, "\n")

	cfg_file:close()


	box:send_stimulation(1, OVTK_StimulationId_TrainCompleted, box:get_current_time() + 0.2, 0)
end
