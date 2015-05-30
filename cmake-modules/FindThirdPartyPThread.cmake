# ---------------------------------
# Finds third party pthread lib
# Adds library to target
# Adds include path
# ---------------------------------

IF(UNIX)

	FIND_LIBRARY(LIB_STANDARD_MODULE_PTHREAD pthread)
	IF(LIB_STANDARD_MODULE_PTHREAD)
		MESSAGE(STATUS "  Found pthread...")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_PTHREAD})
	ELSE(LIB_STANDARD_MODULE_PTHREAD)
		MESSAGE(STATUS "  FAILED to find pthread...")
		ADD_DEFINITIONS(-DTARGET_HAS_PThread)
	ENDIF(LIB_STANDARD_MODULE_PTHREAD)

ENDIF(UNIX)
