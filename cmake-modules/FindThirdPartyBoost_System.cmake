# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------


IF(UNIX)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_System NAMES "boost_system-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)

	IF(LIB_Boost_System)
		MESSAGE(STATUS "    [  OK  ] lib ${LIB_Boost_System}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_System} )
	ELSE(LIB_Boost_System)
		# Fedora 20 and Ubuntu 13.10,14.04 have no more multi-thread boost libs ( *-mt ) so try if there are non -mt libs to link
		FIND_LIBRARY(LIB_Boost_System NAMES "boost_system" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_Boost_System NAMES "boost_system" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)
		IF(LIB_Boost_System)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_Boost_System}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_System})
		ELSE(LIB_Boost_System)
			MESSAGE(STATUS "    [FAILED] lib boost_system-mt")	
			MESSAGE(STATUS "    [FAILED] lib boost_system")
		ENDIF(LIB_Boost_System)
	ENDIF(LIB_Boost_System)
ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("system" ${OV_WIN32_BOOST_VERSION})
ENDIF(WIN32)
