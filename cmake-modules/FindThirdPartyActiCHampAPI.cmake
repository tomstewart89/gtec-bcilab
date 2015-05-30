# /*
 # * FindThirdPartyActiCHampAPI.cmake
 # *
 # * Copyright (c) 2012, Mensia Technologies SA. All rights reserved.
 # * -- Rights transferred to Inria, contract signed 21.11.2014
 # *
 # */

# ---------------------------------
# Finds ActiCHamp library
# Adds library to target
# Adds include path
# ---------------------------------
IF(WIN32)
	FIND_PATH(PATH_ActiCHampAPI actiCHamp.h PATHS "C:/Program Files/actichamp" "C:/Program Files (x86)/actichamp" ${OV_CUSTOM_DEPENDENCIES_PATH})
	IF(PATH_ActiCHampAPI)
		MESSAGE(STATUS "  Found actiCHamp API...")
		INCLUDE_DIRECTORIES(${PATH_ActiCHampAPI})

		FIND_LIBRARY(LIB_ActiCHampAPI actiCHamp_x86 PATHS ${PATH_ActiCHampAPI} )
		IF(LIB_ActiCHampAPI)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_ActiCHampAPI}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_ActiCHampAPI} )
		ELSE(LIB_ActiCHampAPI)
			MESSAGE(STATUS "    [FAILED] lib actiCHamp")
		ENDIF(LIB_ActiCHampAPI)

		FIND_FILE(FIRMWARE_ActiCHampAPI ActiChamp.bit PATHS ${PATH_ActiCHampAPI} )
		IF(FIRMWARE_ActiCHampAPI)
			MESSAGE(STATUS "    [  OK  ] firmware ${FIRMWARE_ActiCHampAPI}")
		ELSE(FIRMWARE_ActiCHampAPI)
			MESSAGE(STATUS "    [FAILED] firmware actiCHamp")
		ENDIF(FIRMWARE_ActiCHampAPI)

		# Copy the DLL file at install
		INSTALL(PROGRAMS "${PATH_ActiCHampAPI}/actiCHamp_x86.dll" DESTINATION "bin")

		# Copy the firmware file at install
		INSTALL(PROGRAMS "${FIRMWARE_ActiCHampAPI}" DESTINATION "bin")
		
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyActiCHampAPI)
	ELSE(PATH_ActiCHampAPI)
		MESSAGE(STATUS "  FAILED to find actiCHamp API (optional) - cmake looked in 'C:/Program Files/actichamp' and 'C:/Program Files (x86)/actichamp'")
	ENDIF(PATH_ActiCHampAPI)
ENDIF(WIN32)
