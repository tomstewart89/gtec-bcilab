# ---------------------------------
# Finds the ENOBIO API & library
# Adds library to target
# Adds include path
# ---------------------------------
IF(WIN32)
	SET(ENOBIOAPI_DIR ${OV_CUSTOM_DEPENDENCIES_PATH}/enobio3g/enobio3g)

  	FIND_PATH(PATH_ENOBIOAPI enobio3g.h PATHS ${ENOBIOAPI_DIR})
	IF(NOT PATH_ENOBIOAPI)
		MESSAGE(STATUS "  FAILED to find ENOBIO API - cmake looked in '${ENOBIOAPI_DIR}', skipping Enobio.")
		RETURN()
	ENDIF(NOT PATH_ENOBIOAPI)
	
	MESSAGE(STATUS "  Found ENOBIO API...")

	FIND_LIBRARY(LIB_ENOBIOAPI Enobio3GAPI PATHS ${PATH_ENOBIOAPI}/../MSVC/ )
	IF(NOT LIB_ENOBIOAPI)
		MESSAGE(STATUS "    [FAILED] Enobio libs not found, skipping Enobio.")	
		RETURN()
	ENDIF(NOT LIB_ENOBIOAPI)
	
	INCLUDE_DIRECTORIES(${PATH_ENOBIOAPI})						
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_ENOBIOAPI} )
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyEnobioAPI)	
			
	MESSAGE(STATUS "    [  OK  ] lib ${LIB_ENOBIOAPI}")

ENDIF(WIN32)

# For now, the Linux building of Enobio3G is disabled as it has not been tested with a recent Neuroelectrics lib.
IF(UNIX)
	MESSAGE(STATUS "  Skipped Enobio3G, its work in progress.")
	RETURN()
ENDIF(UNIX)

IF(UNIX)
	SET(QTCORE_INCLUDE_PREFIX /usr/share/qt4/include/)
	SET(QTCORE_LIB_PREFIX /usr/lib/x86_64-linux-gnu/)
	
	FIND_PATH(PATH_ENOBIOAPI enobio3g.h PATHS $ENV{OpenViBE_dependencies} ${OV_BASE_DIR}/contrib/plugins/server-drivers/enobio3G/Enobio3GAPI.linux/)
	IF(PATH_ENOBIOAPI)
		MESSAGE(STATUS "  Found ENOBIO API...")
		INCLUDE_DIRECTORIES(${PATH_ENOBIOAPI})
		FIND_PATH(PATH_QTCORE_INCLUDE QtCore/QtCore ${QTCORE_INCLUDE_PREFIX})
		IF(PATH_QTCORE_INCLUDE)
		  MESSAGE(STATUS "    [  OK  ] QtCore include ${PATH_QTCORE_INCLUDE}/")
		  INCLUDE_DIRECTORIES(${PATH_QTCORE_INCLUDE}/)
		ELSE(PATH_QTCORE_INCLUDE)
		  MESSAGE(STATUS "    FAILED TO FIND QtCore include PATH")
		ENDIF(PATH_QTCORE_INCLUDE)
		FIND_LIBRARY(LIB_ENOBIOAPI Enobio3GAPI PATHS ${PATH_ENOBIOAPI}/libs/ )
		IF(LIB_ENOBIOAPI)
			MESSAGE(STATUS "    [  OK  ] lib ${LIB_ENOBIOAPI}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_ENOBIOAPI} )
		ELSE(LIB_ENOBIOAPI)
			MESSAGE(STATUS "    [FAILED] lib ENOBIO")
		ENDIF(LIB_ENOBIOAPI)

		FIND_LIBRARY(LIB_QT QtCore ${QTCORE_LIB_PREFIX})
		IF(LIB_QT)
		  MESSAGE(STATUS "    [  OK  ] lib ${LIB_QT}")
		  TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${LIB_QT} -lQtCore)
		ELSE(LIB_QT)
		  MESSAGE(STATUS "    [  FAILED  ] lib QT ${QTCORE_LIB_PREFIX}")
		ENDIF(LIB_QT)
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyEnobioAPI)
		
		# Copying the DLL file at postbuild
		ADD_CUSTOM_COMMAND(
				TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND}
				ARGS -E copy "${LIB_ENOBIOAPI}" "${PROJECT_SOURCE_DIR}/bin"
				COMMENT "      --->   Copying lib file ${LIB_ENOBIOAPI} for the Neuroelectrics Enobio driver."
			VERBATIM)
	ELSE(PATH_ENOBIOAPI)
		MESSAGE(STATUS "  FAILED to find ENOBIO API - cmake looked in $ENV{OpenViBE_dependencies} and in ${OV_BASE_DIR}/contrib/plugins/server-drivers/enobio3G/Enobio3GAPI/")
	ENDIF(PATH_ENOBIOAPI)
ENDIF(UNIX)
