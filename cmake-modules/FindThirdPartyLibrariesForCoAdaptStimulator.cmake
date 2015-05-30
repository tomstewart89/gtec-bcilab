# ---------------------------------
# Finds dependencies required by the external stimulator
# Adds library to target
# Adds include path
# ---------------------------------

SET(foundIncludeDir 1)	   # true, if all required headers have been found
SET(required_libs_found 1) # true, if all libs which have headers have been found
	
IF(MSVC90)
	MESSAGE(STATUS "CoAdapt P300 requires at least VS 2010\n")
	RETURN()
ENDIF(MSVC90)

#GL is natively installed on our Linux pc's and comes with the installation of visual studio. In that case however, one has to make sure the SDK is in the INCLUDE environment variable
IF(WIN32)
	FIND_PATH(PATH_GL GL\\gl.h)
ELSE(WIN32)
	FIND_PATH(PATH_GL GL/gl.h)
ENDIF(WIN32)
IF(PATH_GL)
	MESSAGE(STATUS "  Found OpenGL in ${PATH_GL}" )
	INCLUDE_DIRECTORIES(${PATH_GL})
ELSE(PATH_GL)
	MESSAGE(STATUS "  FAILED to find OpenGL" )
	SET(foundIncludeDir 0)
ENDIF(PATH_GL)

#GLFW (for initializing OpenGL context, lighter than SDL)
IF(WIN32)
	FIND_PATH(PATH_GLFW include/GLFW/glfw3.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH} ${OV_CUSTOM_DEPENDENCIES_PATH}/GLFW )
	IF(PATH_GLFW)
		MESSAGE(STATUS "  Found GLFW3 in ${PATH_GLFW}" )
		INCLUDE_DIRECTORIES(${PATH_GLFW}/include)
	ELSE(PATH_GLFW)
		MESSAGE(STATUS "  FAILED to find GLFW3" )
		SET(foundIncludeDir 0)
	ENDIF(PATH_GLFW)
ELSE(WIN32)
	INCLUDE("FindThirdPartyPkgConfig")
	pkg_check_modules(GLFW glfw3)
	IF(GLFW_FOUND)
		MESSAGE(STATUS "  Found GLFW3 in ${GLFW_INCLUDE_DIRS}" )
		INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIRS})
	ELSE(GLFW_FOUND)
		# Try ov custom dependencies...
		FIND_PATH(PATH_GLFW include/GLFW/glfw3.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH} ${OV_CUSTOM_DEPENDENCIES_PATH}/GLFW )	
		IF(PATH_GLFW)
			MESSAGE(STATUS "  Found GLFW3 in ${PATH_GLFW}" )	
			INCLUDE_DIRECTORIES(${PATH_GLFW}/include)			
			SET(GLFW_LIBRARY_DIR ${PATH_GLFW}/lib)			
			SET(GLFW_LIBRARIES glfw)
		ELSE(PATH_GLFW)
			MESSAGE(STATUS "  FAILED to find GLFW3")
			SET(foundIncludeDir 0)
		ENDIF(PATH_GLFW)
	ENDIF(GLFW_FOUND)
ENDIF(WIN32)

#include windows library inpout32 for writing to parallel port
IF(WIN32)
	FIND_PATH(PATH_INPOUT32 include/inpout32.h ${OV_CUSTOM_DEPENDENCIES_PATH}/inpout32/)
	IF(PATH_INPOUT32)
		MESSAGE(STATUS "  Found inpout32 in ${PATH_INPOUT32}")
		INCLUDE_DIRECTORIES(${PATH_INPOUT32}/include)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyInpout)
	ELSE(PATH_INPOUT32)
		MESSAGE(STATUS "  FAILED to find inpout32 (optional), parallel port tagging support disabled")	
		# optional 
	ENDIF(PATH_INPOUT32)
ENDIF(WIN32)

FIND_PATH(PATH_PRESAGE presage.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/include/ ${OV_CUSTOM_DEPENDENCIES_PATH}/include/presage/ ${OV_CUSTOM_DEPENDENCIES_PATH}/presage/include)
IF(PATH_PRESAGE)
	MESSAGE(STATUS "  Found Presage in ${PATH_PRESAGE}")
	INCLUDE_DIRECTORIES(${PATH_PRESAGE})
	SET(name_libpresage presage)
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyPresage)
ELSE(PATH_PRESAGE)
	MESSAGE(STATUS "  FAILED to find presage (optional), P300 word prediction disabled")
	# optional
ENDIF(PATH_PRESAGE)

IF(foundIncludeDir)
	MESSAGE(STATUS  "  Found required headers for External P300 Stimulator...")
	IF(WIN32)
		FIND_LIBRARY(LIB_GLFW glfw3 PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/GLFW/lib)
		IF(LIB_GLFW)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_GLFW}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_GLFW})
		ELSE(LIB_GLFW)
			MESSAGE(STATUS "    [FAILED] lib GLFW")
			SET(required_libs_found 0)#false
		ENDIF(LIB_GLFW)
		
		IF(PATH_PRESAGE)
			FIND_LIBRARY(LIB_presage libpresage-1 PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/presage/lib)		
			IF(LIB_presage)
				MESSAGE(STATUS "    [  OK  ] lib  ${LIB_presage}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_presage})
			ELSE( LIB_presage)
				MESSAGE(STATUS "    [FAILED] lib presage")
				SET(required_libs_found 0) # since we found the header, we require the lib
			ENDIF(LIB_presage)
		ENDIF(PATH_PRESAGE)

		FIND_LIBRARY(LIB_GL OpenGL32)		
		IF(LIB_GL)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_GL}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_GL})
		ELSE(LIB_GL)
			MESSAGE(STATUS "    [FAILED] lib GL")
			SET(required_libs_found 0)#false
		ENDIF(LIB_GL)

		#inpout32.lib
		IF(PATH_INPOUT32)
			FIND_LIBRARY(LIB_INPOUT32 NAMES "inpout32.lib" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/inpout32/lib/)
			IF(LIB_INPOUT32)
				MESSAGE(STATUS "    [  OK  ] lib ${LIB_INPOUT32}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_INPOUT32} )
			ELSE(LIB_INPOUT32)
				MESSAGE(STATUS "    [FAILED] lib inpout32")
				SET(required_libs_found 0)  # since we found the header, we require the lib			
			ENDIF(LIB_INPOUT32)	
		ENDIF(PATH_INPOUT32)
	ELSE(WIN32)
		FIND_LIBRARY(LIB_GL GL)
		IF(LIB_GL)
			MESSAGE(STATUS "    [  OK  ] GL ${LIB_GL}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_GL})
		ELSE(LIB_GL)
			MESSAGE(STATUS "    [FAILED] lib GL")
			SET(required_libs_found 0)#false
		ENDIF(LIB_GL)

		FIND_LIBRARY(LIB_GLU GLU)
		IF(LIB_GLU)
			MESSAGE(STATUS "    [  OK  ] GLU ${LIB_GLU}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_GLU})
		ELSE(LIB_GUL)
			MESSAGE(STATUS "    [FAILED] lib GLU")
			SET(required_libs_found 0)#false
		ENDIF(LIB_GLU)

		FOREACH(GLFW_LIB ${GLFW_LIBRARIES})
			SET(GLFW_LIB1 "GLFW_LIB1-NOTFOUND")
			FIND_LIBRARY(GLFW_LIB1 NAMES ${GLFW_LIB} PATHS ${GLFW_LIBRARY_DIRS} ${GLFW_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(GLFW_LIB1 NAMES ${GLFW_LIB} PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib/)
			FIND_LIBRARY(GLFW_LIB1 NAMES ${GLFW_LIB})
			IF(GLFW_LIB1)
				MESSAGE(STATUS "    [  OK  ] Third party lib ${GLFW_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GLFW_LIB1})
			ELSE(GLFW_LIB1)
				MESSAGE(STATUS "    [FAILED] Third party lib ${GLFW_LIB}")
				SET(required_libs_found 0)#false
			ENDIF(GLFW_LIB1)
		ENDFOREACH(GLFW_LIB)

		IF(PATH_PRESAGE)
			FIND_LIBRARY(LIB_PRESAGE ${name_libpresage} PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib/)
			IF(LIB_PRESAGE)
				MESSAGE(STATUS "    [  OK  ] Presage ${LIB_PRESAGE}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_PRESAGE})
			ELSE(LIB_PRESAGE)
				MESSAGE(STATUS "    [FAILED] Presage")
				SET(required_libs_found 0) # since we found the header, we require the lib
			ENDIF(LIB_PRESAGE) 
		ENDIF(PATH_PRESAGE)

		#pthread and rt seemed to be necessary on linux when using boost interprocess communication
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread rt)	
	ENDIF(WIN32)

	IF(required_libs_found)
		MESSAGE(STATUS  "  Ok, found all the required headers and libs for the External P300 Stimulator...")	
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyModulesForCoAdaptStimulator -D_GNU_SOURCE=1 -D_REENTRANT)
	ENDIF(required_libs_found)
ENDIF(foundIncludeDir)

IF(NOT (foundIncludeDir AND required_libs_found) )
	MESSAGE(STATUS  "  Did not find all required headers and/or libs for the External P300 Stimulator...")
ENDIF(NOT (foundIncludeDir AND required_libs_found) )	


