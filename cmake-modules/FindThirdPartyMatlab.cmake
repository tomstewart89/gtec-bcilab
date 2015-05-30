# ---------------------------------
# Finds Matlab toolkit
# ---------------------------------

# Clear cached variables, otherwise repeated builds lead to trouble
SET(Matlab_EXECUTABLE "Matlab_EXECUTABLE-NOTFOUND")
SET(Matlab_INCLUDE "Matlab_INCLUDE-NOTFOUND")
SET(Matlab_ROOT "Matlab_ROOT-NOTFOUND")

# See if we can locate the matlab executable 
IF (WIN32)
	FIND_PROGRAM(Matlab_EXECUTABLE MATLAB)
ENDIF(WIN32)
IF (UNIX)
	FIND_PROGRAM(Matlab_EXECUTABLE matlab)
	
	IF(NOT Matlab_EXECUTABLE)
		# Alternative way to try to find matlab
		FILE(GLOB_RECURSE Executable_Candidates1 "/usr/local/matlab*/matlab")
		FILE(GLOB_RECURSE Executable_Candidates2 "/usr/local/MATLAB*/matlab")
		SET(Executable_Candidates ${Executable_Candidates1} ${Executable_Candidates2})
		
		IF(Executable_Candidates) 
			LIST(GET Executable_Candidates 0 Matlab_EXECUTABLE)
		ENDIF(Executable_Candidates)
	ENDIF(NOT Matlab_EXECUTABLE)
ENDIF(UNIX)

# Figure out the paths to libs and includes
IF(Matlab_EXECUTABLE)
	# MESSAGE(STATUS "Have Matlab_EXECUTABLE ${Matlab_EXECUTABLE}")
	# Try relative to the executable path
	GET_FILENAME_COMPONENT(Matlab_ROOT ${Matlab_EXECUTABLE} PATH)
	IF(Matlab_ROOT)
		# MESSAGE(STATUS "Have Matlab_ROOT ${Matlab_ROOT}")
		SET(Matlab_ROOT ${Matlab_ROOT}/../)
		# MESSAGE(STATUS " -> ${Matlab_ROOT}")	
		FIND_PATH(Matlab_INCLUDE "mex.h" PATHS ${Matlab_ROOT}/extern/include ${Matlab_ROOT}/extern/include/extern)
	ENDIF(Matlab_ROOT)

	# matlab executable path might have been pointing to a symbolic link elsewhere, try something else
	IF((NOT Matlab_INCLUDE) AND UNIX)
		EXECUTE_PROCESS(COMMAND matlab -e COMMAND grep "^MATLAB=" COMMAND sed "s/^MATLAB=//g" COMMAND tr "\n" "/" 
				OUTPUT_VARIABLE Matlab_ROOT)
		FIND_PATH(Matlab_INCLUDE "mex.h" PATHS ${Matlab_ROOT}/extern/include ${Matlab_ROOT}/extern/include/extern)
	ENDIF((NOT Matlab_INCLUDE) AND UNIX)

	IF(Matlab_INCLUDE)
		# MESSAGE(STATUS "Have Matlab_INCLUDE ${Matlab_INCLUDE}")
		IF(UNIX)
			SET(Matlab_LIBRARIES mex mx eng)
			IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
				SET(Matlab_LIB_DIRECTORIES ${Matlab_ROOT}/bin/glnx86)
			ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
				SET(Matlab_LIB_DIRECTORIES ${Matlab_ROOT}/bin/glnxa64)
			ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
		ENDIF(UNIX)
		IF(WIN32)
			SET(Matlab_LIBRARIES libmex libmx libeng) #mclmcrrt
			SET(Matlab_LIB_DIRECTORIES ${Matlab_ROOT}/extern/lib/win32/microsoft)
			# for delayed importation on windows
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} Delayimp )
			SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:libeng.dll /DELAYLOAD:libmx.dll")
			# /DELAYLOAD:libmex.dll /DELAYLOAD:mclmcrrt.dll --> useless, no import
		ENDIF(WIN32)
		SET(Matlab_FOUND TRUE)
	ENDIF(Matlab_INCLUDE)

ENDIF(Matlab_EXECUTABLE)

IF(Matlab_FOUND)
	MESSAGE(STATUS "  Found Matlab in [${Matlab_ROOT}]")
	SET(Matlab_LIB_FOUND TRUE)
	INCLUDE_DIRECTORIES(${Matlab_INCLUDE})
	
	FOREACH(Matlab_LIB ${Matlab_LIBRARIES})
		SET(Matlab_LIB1 "Matlab_LIB1-NOTFOUND")
		FIND_LIBRARY(Matlab_LIB1 NAMES ${Matlab_LIB} PATHS ${Matlab_LIB_DIRECTORIES} NO_DEFAULT_PATH)
		FIND_LIBRARY(Matlab_LIB1 NAMES ${Matlab_LIB})
		IF(Matlab_LIB1)
			MESSAGE(STATUS "	[  OK  ] Third party lib ${Matlab_LIB1}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Matlab_LIB1})
		ELSE(Matlab_LIB1)
			MESSAGE(STATUS "	[FAILED] Third party lib ${Matlab_LIB}")
			SET(Matlab_LIB_FOUND FALSE)
		ENDIF(Matlab_LIB1)
	ENDFOREACH(Matlab_LIB)
	IF(Matlab_LIB_FOUND)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyMatlab)
	ELSE(Matlab_LIB_FOUND)
		MESSAGE(STATUS "  FAILED to find Matlab Libs, the plugins won't be built. Please ensure you have a valid MATLAB installation (32 bits only).")
	ENDIF(Matlab_LIB_FOUND)
ELSE(Matlab_FOUND)
	MESSAGE(STATUS "  FAILED to find Matlab...")
ENDIF(Matlab_FOUND)

