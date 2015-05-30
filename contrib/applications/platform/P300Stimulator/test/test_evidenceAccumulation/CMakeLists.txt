PROJECT(test_evidenceaccumulation)

IF(WIN32)
	ADD_DEFINITIONS(-DTARGET_OS_Windows)
ENDIF(WIN32)
IF(UNIX)
	ADD_DEFINITIONS(-DTARGET_OS_Linux)
ENDIF(UNIX)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)

FILE(GLOB_RECURSE source_files ../../src/*.cpp ../../src/*.h)
ADD_EXECUTABLE(${PROJECT_NAME} ${source_files} test_evidenceAccumulation.cpp)

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


#INCLUDE_DIRECTORIES(../src)
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
ENDIF(UNIX)


# Unfortunately we need to install the tests as any application to find .dll/.so files
# on both Windows and Linux.
OV_INSTALL_LAUNCH_SCRIPT(${PROJECT_NAME})
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
	
