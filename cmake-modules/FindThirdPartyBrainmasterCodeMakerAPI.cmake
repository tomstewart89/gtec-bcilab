# ---------------------------------
# Finds BrainmasterCodeMakerAPI
# Adds library to target
# Adds include path
# ---------------------------------
IF(WIN32)
	# note that ampersands in paths below passed through cmake mess up Visual Studio IDE. If you get this problem, rename the dll folder under Program Files/
	FIND_PATH(PATH_BrainmasterCodeMakerAPI CMKRDLLU.H PATHS 
		"C:/Program Files/Atlantis and Discovery SW DLL 1-11-12" 
		"C:/Program Files (x86)/Atlantis and Discovery SW DLL 1-11-12" 	
		"C:/Program Files/Atlantis & Discovery SW DLL 1-11-12" 
		"C:/Program Files (x86)/Atlantis & Discovery SW DLL 1-11-12" 
		${OV_CUSTOM_DEPENDENCIES_PATH})
	IF(PATH_BrainmasterCodeMakerAPI)
		MESSAGE(STATUS "  Found Brainmaster Code Maker API...")
		INCLUDE_DIRECTORIES(${PATH_BrainmasterCodeMakerAPI})

		FOREACH(LIB_BrainmasterCodeMakerAPI bmrcm.lib ovbmrcm.lib)
			SET(LIB_BrainmasterCodeMakerAPI1 "LIB_BrainmasterCodeMakerAPI1-NOTFOUND")
			FIND_LIBRARY(LIB_BrainmasterCodeMakerAPI1 NAMES ${LIB_BrainmasterCodeMakerAPI} PATHS ${PATH_BrainmasterCodeMakerAPI} NO_DEFAULT_PATH)
			FIND_LIBRARY(LIB_BrainmasterCodeMakerAPI1 NAMES ${LIB_BrainmasterCodeMakerAPI})
			IF(LIB_BrainmasterCodeMakerAPI1)
				MESSAGE(STATUS "    [  OK  ] Third party lib ${LIB_BrainmasterCodeMakerAPI1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_BrainmasterCodeMakerAPI1})
			ELSE(LIB_BrainmasterCodeMakerAPI1)
				MESSAGE(STATUS "    [FAILED] Third party lib ${LIB_BrainmasterCodeMakerAPI}")
			ENDIF(LIB_BrainmasterCodeMakerAPI1)
		ENDFOREACH(LIB_BrainmasterCodeMakerAPI)

		# Copy the DLL file at install
		INSTALL(PROGRAMS "${PATH_BrainmasterCodeMakerAPI}/bmrcm.dll" DESTINATION "bin")

		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyBrainmasterCodeMakerAPI)
	ELSE(PATH_BrainmasterCodeMakerAPI)
		MESSAGE(STATUS "  FAILED to find Brainmaster Code Maker API (optional)")
	ENDIF(PATH_BrainmasterCodeMakerAPI)
ENDIF(WIN32)
