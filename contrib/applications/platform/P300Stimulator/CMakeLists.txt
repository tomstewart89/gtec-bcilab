
IF(OV_DISABLE_GTK)
  MESSAGE(STATUS  "Skipping CoAdapt P300 Stimulator, no GTK")
  RETURN()
ENDIF(OV_DISABLE_GTK)

PROJECT(openvibe-coadapt-p300-stimulator)

SET(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
SET(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
SET(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h)
ADD_EXECUTABLE(${PROJECT_NAME} ${source_files} ovexP300Visualiser.h ovexP300Visualiser.cpp)

INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")
INCLUDE("FindOpenViBEToolkit")
INCLUDE("FindOpenViBEModuleEBML")
INCLUDE("FindOpenViBEModuleXML")
INCLUDE("FindOpenViBEModuleSystem")
INCLUDE("FindOpenViBEModuleXML")
INCLUDE("FindThirdPartyBoost")
INCLUDE("FindThirdPartyLibrariesForCoAdaptStimulator")
INCLUDE("FindThirdPartyGTK")#to use gtk package content i.e cairo pango fretype

#so that boost won't need to link against DateTime when using the interprocess communication library
ADD_DEFINITIONS(-DBOOST_DATE_TIME_NO_LIB)

#compile output timing to file for tests
IF(OV_COMPILE_TESTS)
        ADD_DEFINITIONS(-DOUTPUT_TIMING)
ENDIF(OV_COMPILE_TESTS)
ADD_SUBDIRECTORY("test")#to compile some test executables
# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compilator
# ---------------------------------
IF(WIN32)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
        ADD_DEFINITIONS(-DTARGET_OS_Windows)
        ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
        ADD_DEFINITIONS(-DTARGET_COMPILER_VisualStudio)
ENDIF(WIN32)
IF(UNIX)
        # ADD_DEFINITIONS(-fvisibility=hidden) # This flag should be present... man gcc
        ADD_DEFINITIONS(-g)
        ADD_DEFINITIONS(-fnon-call-exceptions)
        ADD_DEFINITIONS(-DTARGET_OS_Linux)
        ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
        ADD_DEFINITIONS(-DTARGET_COMPILER_GCC)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} rt)
ENDIF(UNIX)

# ----------------------
# Generate launch script
# ----------------------
OV_INSTALL_LAUNCH_SCRIPT("openvibe-coadapt-p300-stimulator")

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

# Due to their size, the presage dictionaries are for now to be installed by hand by the user. 
# If we allowed install here, the packaging would pull the dictionaries to the resulting exe/deb
# making it very large. 

# IF(LIB_PRESAGE)
#	INSTALL(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe  PATTERN ".svn" EXCLUDE)
#ELSE(LIB_PRESAGE)
	INSTALL(DIRECTORY share/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe  PATTERN ".svn" EXCLUDE PATTERN "*.db" EXCLUDE)
#ENDIF(LIB_PRESAGE)

#INSTALL(DIRECTORY src/   DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE PATTERN "doc" EXCLUDE)
