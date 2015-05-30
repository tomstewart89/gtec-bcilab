# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------


IF(UNIX)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)
	
	IF(LIB_Boost_Regex)
		MESSAGE(STATUS "    [  OK  ] lib ${LIB_Boost_Regex}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Regex} )
	ELSE(LIB_Boost_Regex)
		# Fedora 20 and Ubuntu 13.10,14.04 have no more multi-thread boost libs ( *-mt ) so try if there are non -mt libs to link
		FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_Boost_Regex NAMES "boost_regex" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)
		IF(LIB_Boost_Regex)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_Boost_Regex}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Regex})
		ELSE(LIB_Boost_Regex)
			MESSAGE(STATUS "    [FAILED] lib boost_regex-mt")	
			MESSAGE(STATUS "    [FAILED] lib boost_regex")
		ENDIF(LIB_Boost_Regex)
	ENDIF(LIB_Boost_Regex)
ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("regex" ${OV_WIN32_BOOST_VERSION})
ENDIF(WIN32)
