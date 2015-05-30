# ---------------------------------
# Finds Ogre3DTerrain toolkit
#
# Sets Ogre3DTerrain_FOUND
# Sets Ogre3DTerrain_LIBRARIES
# Sets Ogre3DTerrain_LIBRARY_DIRS
# Sets Ogre3DTerrain_LDFLAGS
# Sets Ogre3DTerrain_LDFLAGS_OTHERS
# Sets Ogre3DTerrain_INCLUDE_DIRS
# Sets Ogre3DTerrain_CFLAGS
# Sets Ogre3DTerrain_CFLAGS_OTHERS
#
# Adds library to target
# Adds include path
# ---------------------------------

IF(OV_DISABLE_OGRE)
	MESSAGE(STATUS "  SKIPPED Ogre3D (Terrain), disabled, no 3D ...")
	RETURN()
ENDIF(OV_DISABLE_OGRE)

IF(WIN32)
	FIND_PATH(PATH_Ogre3DTerrain include/OGRE/Ogre.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/ogre $ENV{OGRE_HOME})
	IF(PATH_Ogre3DTerrain)
		SET(Ogre3DTerrain_FOUND TRUE)
		SET(OIS_FOUND TRUE)
		SET(Ogre3DTerrain_INCLUDE_DIRS ${PATH_Ogre3DTerrain}/include/OGRE ${PATH_Ogre3DTerrain}/include/OIS)

		SET(Ogre3DTerrain_LIBRARIES_RELEASE OgreMain OIS)
		SET(Ogre3DTerrain_LIBRARY_DIRS_RELEASE ${PATH_Ogre3DTerrain}/lib/Release)
						
		SET(Ogre3DTerrain_LIBRARIES_DEBUG OgreMain_d OIS_d)
		SET(Ogre3DTerrain_LIBRARY_DIRS_DEBUG ${PATH_Ogre3DTerrain}/lib/Debug)
	ENDIF(PATH_Ogre3DTerrain)
ENDIF(WIN32)

IF(UNIX)
	INCLUDE("FindThirdPartyPkgConfig")
	pkg_check_modules(Ogre3DTerrain OGRE-Terrain)
	pkg_check_modules(OIS OIS)
ENDIF(UNIX)

IF(Ogre3DTerrain_FOUND AND OIS_FOUND)
	MESSAGE(STATUS "  Found Ogre3DTerrain/OIS...")
	INCLUDE_DIRECTORIES(${OIS_INCLUDE_DIRS} ${Ogre3DTerrain_INCLUDE_DIRS})
	ADD_DEFINITIONS(${OIS_CFLAGS} ${Ogre3DTerrain_CFLAGS})
	ADD_DEFINITIONS(${OIS_CFLAGS_OTHERS} ${Ogre3DTerrain_CFLAGS_OTHERS})
	# MESSAGE(STATUS "A ${OIS_CFLAGS} ${Ogre3DTerrain_CFLAGS} B ${OIS_CFLAGS_OTHERS} ${Ogre3DTerrain_CFLAGS_OTHERS} C ${Ogre3DTerrain_LIBRARIES_RELEASE} D ${Ogre3DTerrain_LIBRARIES_DEBUG}")

	IF(UNIX) 
		FOREACH(Ogre3DTerrain_LIB ${Ogre3DTerrain_LIBRARIES} ${OIS_LIBRARIES})
			SET(Ogre3DTerrain_LIB1 "Ogre3DTerrain_LIB1-NOTFOUND")
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB} PATHS ${Ogre3DTerrain_LIBRARY_DIRS} ${Ogre3DTerrain_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB})
			IF(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [  OK  ] Third party lib ${Ogre3DTerrain_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Ogre3DTerrain_LIB1})
			ELSE(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [FAILED] Third party lib ${Ogre3DTerrain_LIB}")
			ENDIF(Ogre3DTerrain_LIB1)
		ENDFOREACH(Ogre3DTerrain_LIB)
	ENDIF(UNIX)
	IF(WIN32)
		FOREACH(Ogre3DTerrain_LIB ${Ogre3DTerrain_LIBRARIES_RELEASE})
			SET(Ogre3DTerrain_LIB1 "Ogre3DTerrain_LIB1-NOTFOUND")
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB} PATHS ${Ogre3DTerrain_LIBRARY_DIRS_RELEASE} ${Ogre3DTerrain_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB})
			IF(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [  OK  ] Third party lib ${Ogre3DTerrain_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${Ogre3DTerrain_LIB1})
			ELSE(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [FAILED] Third party lib ${Ogre3DTerrain_LIB}")
			ENDIF(Ogre3DTerrain_LIB1)
		ENDFOREACH(Ogre3DTerrain_LIB)
		
		FOREACH(Ogre3DTerrain_LIB ${Ogre3DTerrain_LIBRARIES_DEBUG})
			SET(Ogre3DTerrain_LIB1 "Ogre3DTerrain_LIB1-NOTFOUND")
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB} PATHS ${Ogre3DTerrain_LIBRARY_DIRS_DEBUG} ${Ogre3DTerrain_LIBDIR} NO_DEFAULT_PATH)
			FIND_LIBRARY(Ogre3DTerrain_LIB1 NAMES ${Ogre3DTerrain_LIB})
			IF(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [  OK  ] Third party lib ${Ogre3DTerrain_LIB1}")
				TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${Ogre3DTerrain_LIB1})
			ELSE(Ogre3DTerrain_LIB1)
				MESSAGE(STATUS "    [FAILED] Third party lib ${Ogre3DTerrain_LIB}")
			ENDIF(Ogre3DTerrain_LIB1)
		ENDFOREACH(Ogre3DTerrain_LIB)
	ENDIF(WIN32)
	
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyOgre3DTerrain)
ELSE(Ogre3DTerrain_FOUND AND OIS_FOUND)
	MESSAGE(STATUS "  FAILED to find Ogre3DTerrain/OIS...")
ENDIF(Ogre3DTerrain_FOUND AND OIS_FOUND)


