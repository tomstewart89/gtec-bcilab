#
# The gMobilab driver (Linux) was contributed by Lucie Daubigney from Supelec Metz
#
# Windows-compatibility added by Jussi T. Lindgren / Inria
#

# ---------------------------------
# Finds GTecMobiLabPlus+
# Adds library to target
# Adds include path
# ---------------------------------

# We have the following block to make sure gUSBAmp is not on the system. It will always override 
# mobilab by default until we can figure out how to have both gUSBAmp and gMobilab drivers built 
# in at the same time.
INCLUDE("FindThirdPartyGUSBampCAPI")
IF(OV_ThirdPartyGUSBAmp)
	MESSAGE(STATUS "  NOTE gtec USBAmp has been found, cannot have gMobilab driver in the same executable")
	MESSAGE(STATUS "    [ SKIP ] GMobiLabCAPI ...")
	RETURN()
ENDIF(OV_ThirdPartyGUSBAmp)

IF(WIN32)
	# note that the API must be 32bit with OpenViBE
	FIND_PATH(PATH_GMobiLabCAPI GMobiLabPlus.h PATHS 
		"C:/Program Files/gtec/GMobiLabCAPI/Lib" 
		"C:/Program Files (x86)/gtec/GMobiLabCAPI/Lib" 
		${OV_CUSTOM_DEPENDENCIES_PATH})
	# We need to copy the DLL on install
	FIND_PATH(PATH_GMobiLabDLL gMOBIlabplus.dll PATHS 
		"C:/Windows/System32" 
		"C:/Windows/SysWOW64" 
		${OV_CUSTOM_DEPENDENCIES_PATH})		
	FIND_LIBRARY(LIB_GMobiLabCAPI GMobiLabplus PATHS ${PATH_GMobiLabCAPI}/x86)
		
	IF(PATH_GMobiLabCAPI AND PATH_GMobiLabDLL AND LIB_GMobiLabCAPI)
		MESSAGE(STATUS "  Found GMobiLabCAPI ...")
		MESSAGE(STATUS "    [  OK  ] lib ${LIB_GMobiLabCAPI}")
			
		INCLUDE_DIRECTORIES(${PATH_GMobiLabCAPI})
				
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_GMobiLabCAPI} )
			
		INSTALL(PROGRAMS ${PATH_GMobiLabDLL}/gMOBIlabplus.dll DESTINATION "bin")
		
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyGMobiLabPlusAPI)
		SET(OV_ThirdPartyGMobilab "YES")

	ELSE(PATH_GMobiLabCAPI AND PATH_GMobiLabDLL AND LIB_GMobiLabCAPI)
		MESSAGE(STATUS "  FAILED to find GMobiLabPlusAPI + lib + dll")
	ENDIF(PATH_GMobiLabCAPI AND PATH_GMobiLabDLL AND LIB_GMobiLabCAPI)
ENDIF(WIN32)

IF(UNIX)
	FIND_LIBRARY(gMOBIlabplus_LIBRARY NAMES "gMOBIlabplus" "gmobilabplusapi" PATHS "/usr/lib" "/usr/local/lib")
	IF(gMOBIlabplus_LIBRARY)
		MESSAGE(STATUS "  Found GMobiLabPlusAPI...")
		MESSAGE(STATUS "    [  OK  ] Third party lib ${gMOBIlabplus_LIBRARY}")
		ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyGMobiLabPlusAPI)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${gMOBIlabplus_LIBRARY} )
	ELSE(gMOBIlabplus_LIBRARY)
		MESSAGE(STATUS "  FAILED to find GMobiLabPlusAPI... ")
		MESSAGE(STATUS "   : If it should be found, see that 'gmobilabapi.so' link exists on the fs, with no numeric suffixes in the filename.")
		MESSAGE(STATUS "   : e.g. do 'cd /usr/lib/ ; ln -s libgmobilabplusapi.so.1.12 libgmobilabplusapi.so' ")
	ENDIF(gMOBIlabplus_LIBRARY)
ENDIF(UNIX)
