classes = nil

current_target = nil
target_set = false

score = {}

function initialize(box)
	dofile(box:get_config("${Path_Data}") .. "/plugins/stimulation/lua-stimulator-stim-codes.lua")
	classes = box:get_setting(2)

	for j = 1, classes do
		score[j] = {}
		for i = 1, classes do
			score[j][i] = 0
		end
	end
end

function uninitialize(box)

	tp = 0
	fp = 0
	tn = 0
	fn = 0

	for j = 1, classes do
		
		output = string.format("Target %d : ", j - 1)
		
		for i = 1, classes do
			output = output .. string.format("%d : %3d, ", i - 1, score[j][i])

			if j == 1 and i == 1 then
				tn = tn + score[j][i]
			elseif j ~= 1 and i == 1 then
				fn = fn + score[j][i]
			elseif i == j then
				tp = tp + score[j][i]
			else
				fp = fp + score[j][i]
			end
		end

		box:log("Info", string.format("%s", output))
	end

	box:log("Info", string.format("TP %3d  |  FP %3d", tp, fp))
	box:log("Info", string.format("TN %3d  |  FN %3d", tn, fn))

end

function process(box)

	finished = false

	while not finished do

		time = box:get_current_time()

		while box:get_stimulation_count(1) > 0 do

			s_code, s_date, s_duration = box:get_stimulation(1, 1)
			box:remove_stimulation(1, 1)

			if s_code >= OVTK_StimulationId_Label_00 and s_code <= OVTK_StimulationId_Label_1F then
				current_target = s_code - OVTK_StimulationId_Label_00

			elseif s_code == OVTK_StimulationId_VisualStimulationStart then
				target_set = true

			elseif s_code == OVTK_StimulationId_ExperimentStop then
				target_set = false

			elseif s_code == OVTK_StimulationId_ExperimentStop then
				finished = true
			end
		end

		while box:get_stimulation_count(2) > 0 do

			s_code, s_date, s_duration = box:get_stimulation(2, 1)
			box:remove_stimulation(2, 1)

			if target_set and (s_code >= OVTK_StimulationId_Label_00 and s_code <= OVTK_StimulationId_Label_1F) then

				score[current_target + 1][s_code - OVTK_StimulationId_Label_00 + 1] = score[current_target + 1][s_code - OVTK_StimulationId_Label_00 + 1] + 1

			end
		end

		box:sleep()

	end

end
