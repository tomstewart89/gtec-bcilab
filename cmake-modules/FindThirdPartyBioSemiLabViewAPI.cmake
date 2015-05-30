# ---------------------------------
# Finds BioSemi LabView API
# Adds library to target
# Adds include path
# ---------------------------------
IF(WIN32)
	FIND_PATH(PATH_BioSemiAPI labview_dll.h PATHS 
		"C:/Program Files/BioSemi/Developers_kit/C-code" 
		"C:/Program Files (x86)/BioSemi/Developers_kit/C-code"
		${OV_CUSTOM_DEPENDENCIES_PATH})
	IF(PATH_BioSemiAPI)
		MESSAGE(STATUS "  Found BioSemi LabView API...")
		INCLUDE_DIRECTORIES(${PATH_BioSemiAPI})
		FIND_LIBRARY(LIB_BioSemiAPI Labview_DLL PATHS ${PATH_BioSemiAPI})
		IF(LIB_BioSemiAPI)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_BioSemiAPI}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_BioSemiAPI} )
		ELSE(LIB_BioSemiAPI)
			MESSAGE(STATUS "    [FAILED] lib Labview_DLL")
		ENDIF(LIB_BioSemiAPI)

		# Copy the DLL file at install
		INSTALL(PROGRAMS "${PATH_BioSemiAPI}/Labview_DLL.dll" DESTINATION "bin")

		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyBioSemiAPI)
	ELSE(PATH_BioSemiAPI)
		MESSAGE(STATUS "  FAILED to find BioSemi LabView API")
	ENDIF(PATH_BioSemiAPI)
ENDIF(WIN32)
