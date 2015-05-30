
IF(OV_DISABLE_GTK)
  MESSAGE(STATUS  "Skipping Acquisition Server, no GTK")
  RETURN()
ENDIF(OV_DISABLE_GTK)

PROJECT(openvibe-acquisition-server)

SET(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
SET(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
SET(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

INCLUDE("FindOpenViBEContributions")

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.inl include/*.h)
IF(PATH_OPENVIBE_CONTRIBUTIONS)
	INCLUDE("${CMAKE_SOURCE_DIR}/contrib/common/contribAcquisitionServer.cmake")
ENDIF(PATH_OPENVIBE_CONTRIBUTIONS)

SET(ADDITIONAL_PATH "${CMAKE_SOURCE_DIR}/plugins/server-extensions/")
FILE(GLOB_RECURSE additional_source_files ${ADDITIONAL_PATH}/*.cpp ${ADDITIONAL_PATH}/*.h)
SET(source_files "${source_files};${additional_source_files}")

ADD_EXECUTABLE(${PROJECT_NAME} ${source_files})
INCLUDE_DIRECTORIES("src/drivers")
INCLUDE_DIRECTORIES("include")

# add include paths for non-contrib server plugins
FILE(GLOB children ${ADDITIONAL_PATH}/*)
FOREACH(FILENAME ${children})
	# MESSAGE("Testing ${FILENAME}")
	IF(IS_DIRECTORY ${FILENAME})
		# MESSAGE("Adding ${FILENAME}")
		INCLUDE_DIRECTORIES(${FILENAME})
	ENDIF(IS_DIRECTORY ${FILENAME})
ENDFOREACH(FILENAME)

INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")
INCLUDE("FindOpenViBEToolkit")
INCLUDE("FindOpenViBEModuleEBML")
INCLUDE("FindOpenViBEModuleSocket")
INCLUDE("FindOpenViBEModuleSystem")
INCLUDE("FindOpenViBEModuleFS")
INCLUDE("FindThirdPartyGTK")

# These scripts will also add directives to copy the necessary bin dlls if any
INCLUDE("FindThirdPartyActiCHampAPI")
INCLUDE("FindThirdPartyBioSemiLabViewAPI")
INCLUDE("FindThirdPartyEmotivAPI")
INCLUDE("FindThirdPartyNeXus")
INCLUDE("FindThirdPartyMCS")
INCLUDE("FindThirdPartyMicromed")
INCLUDE("FindThirdPartyNeuroelectricsEnobio3G")
INCLUDE("FindThirdPartyThinkGearAPI")
INCLUDE("FindThirdPartyTMSi")
INCLUDE("FindThirdPartyUSBFirstAmpAPI")

INCLUDE("FindThirdPartyOpenAL")
INCLUDE("FindThirdPartyBoost")
INCLUDE("FindThirdPartyBoost_Thread")
INCLUDE("FindThirdPartyPThreadsVCE2")	# for Windows Fieldtrip

INCLUDE("FindThirdPartyLSL")

IF(PATH_OPENVIBE_CONTRIBUTIONS)
	INCLUDE("${CMAKE_SOURCE_DIR}/contrib/common/contribAcquisitionServerLinkLibs.cmake")
ENDIF(PATH_OPENVIBE_CONTRIBUTIONS)


# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
	#ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
	ADD_DEFINITIONS(-DTARGET_OS_Windows)
	ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ADD_DEFINITIONS(-DTARGET_COMPILER_VisualStudio)
	ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
ENDIF(WIN32)
IF(UNIX)
	# ADD_DEFINITIONS(-fvisibility=hidden) # This flag should be present... man gcc
	ADD_DEFINITIONS(-g)
	ADD_DEFINITIONS(-fnon-call-exceptions)
	ADD_DEFINITIONS(-DTARGET_OS_Linux)
	ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ADD_DEFINITIONS(-DTARGET_COMPILER_GCC)
ENDIF(UNIX)

# ----------------------
# Generate launch script
# ----------------------
OV_INSTALL_LAUNCH_SCRIPT("openvibe-acquisition-server")

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

CONFIGURE_FILE("share/interface.ui-base" "${CMAKE_CURRENT_BINARY_DIR}/interface.ui")

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/interface.ui" DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/acquisition-server)
INSTALL(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/applications/acquisition-server PATTERN ".svn" EXCLUDE PATTERN "*-base" EXCLUDE)

#INSTALL(DIRECTORY src/   DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE PATTERN "doc" EXCLUDE)
