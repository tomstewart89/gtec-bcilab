project(test_ebml_read)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

INCLUDE_DIRECTORIES(../src)
ADD_EXECUTABLE(${PROJECT_NAME} test_ebml_read.cpp)

INCLUDE("FindOpenViBEModuleEBML")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
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
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
		
OV_INSTALL_LAUNCH_SCRIPT(${PROJECT_NAME})

INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

############################################################################################
		
project(test_ebml_write)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

INCLUDE_DIRECTORIES(../src)
ADD_EXECUTABLE(${PROJECT_NAME} test_ebml_write.cpp)

INCLUDE("FindOpenViBEModuleEBML")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
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
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
		
OV_INSTALL_LAUNCH_SCRIPT(${PROJECT_NAME})

INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

############################################################################################

project(test_ebml_tree)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

INCLUDE_DIRECTORIES(../src)
ADD_EXECUTABLE(${PROJECT_NAME} test_ebml_tree.cpp)

INCLUDE("FindOpenViBEModuleEBML")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
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
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
		
OV_INSTALL_LAUNCH_SCRIPT(${PROJECT_NAME})

INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})


		