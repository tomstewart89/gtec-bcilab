IF(WIN32)
	SET(EXT cmd)
	SET(OS_FLAGS "--no-pause")
ELSE(WIN32)
	SET(EXT sh)
	SET(OS_FLAGS "")
ENDIF(WIN32)

# MESSAGE(STATUS "PROC ${SCENARIO_BASE}")

EXECUTE_PROCESS(
	COMMAND "$ENV{OV_BINARY_PATH}/openvibe-designer.${EXT}" ${OS_FLAGS} "--no-gui" "--no-session-management" "--play" "${SCENARIO_BASE}.xml" 
	COMMAND "grep" "TestRunCommand Params:" 
	OUTPUT_VARIABLE OV_RESULT
	OUTPUT_FILE "${SCENARIO_BASE}.txt")

# MESSAGE(STATUS "Result was ${OV_RESULT}")

# RETURN(${OV_RESULT})

# MESSAGE(STATUS "$ENV{OV_BINARY_PATH}/openvibe-designer.${EXT}" ${OS_FLAGS} "--no-gui" "--no-session-management" "--play $ENV{OV_SCENARIO_TO_TEST}" OUTPUT_FILE "$ENV{OV_TEST_NAME}.tmp.txt")



