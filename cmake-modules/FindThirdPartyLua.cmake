# ---------------------------------
# Finds LUA
# Adds library to target
# Adds include path
# ---------------------------------
IF(WIN32)
	FIND_PATH(LUA_INCLUDE_DIR lua.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lua/include NO_DEFAULT_PATH)
ELSEIF(UNIX)
	FIND_PACKAGE(Lua51 REQUIRED)
ENDIF(WIN32)

IF(LUA_INCLUDE_DIR)
	MESSAGE(STATUS "  Found Lua... in '${LUA_INCLUDE_DIR}'")

	IF(WIN32)
		FIND_LIBRARY(LUA_LIBRARIES lua5.1 PATHS ${LUA_INCLUDE_DIR}/../lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LUA_LIBRARIES lua5.1 PATHS ${LUA_INCLUDE_DIR}/../lib)
	ENDIF(WIN32)

	IF(LUA_LIBRARIES)
		MESSAGE(STATUS "	[  OK  ] lib ${LUA_LIBRARIES}")

		INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LUA_LIBRARIES})
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyLua)
	ELSE(LUA_LIBRARIES)
		MESSAGE(STATUS "	[FAILED] lib lua5.1")
	ENDIF(LUA_LIBRARIES)

ELSE(LUA_INCLUDE_DIR)
	MESSAGE(STATUS "  FAILED to find Lua")
ENDIF(LUA_INCLUDE_DIR)
