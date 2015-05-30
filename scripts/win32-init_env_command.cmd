@echo off

REM ########################################################################################################################

if exist "win32-dependencies.cmd" (
	call "win32-dependencies.cmd"
) else (
	echo ERROR: win32-dependencies.cmd not found. Has the dependency installer been run?
	goto terminate
	
)

REM ########################################################################################################################

REM # To force build with VS 2008 with VS 2010 installed as well, set the following to 1
SET SKIP_VS2010=0

SET VSTOOLS=
SET VSCMake=

if %SKIP_VS2010% == 1 (
	echo Visual Studio 2010 detection skipped as requested
) else (
	if exist "%VS100COMNTOOLS%vsvars32.bat" (
		echo Found VS100 tools at "%VS100COMNTOOLS%" ...
		CALL "%VS100COMNTOOLS%vsvars32.bat"
		SET VSCMake=Visual Studio 10
		goto terminate
	)
)

if exist "%VS90COMNTOOLS%vsvars32.bat" (
	echo Found VS90 tools at "%VS90COMNTOOLS%" ...
	CALL "%VS90COMNTOOLS%vsvars32.bat"
	SET VSCMake=Visual Studio 9 2008
	goto terminate
)

echo ######################################################################################
echo ##                                                                                  ##
echo ##  ERROR : Microsoft Visual Studio Common tools initialisation script not found    ##
echo ##                                                                                  ##
echo ######################################################################################
goto terminate

REM #######################################################################################

:terminate
