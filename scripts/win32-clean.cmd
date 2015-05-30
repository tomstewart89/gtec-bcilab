@echo off

REM this script just deletes all the resulting build folders. 
REM You should do make clean manually in VS or make if that is what you want.

SET ARGFOUND=0
SET CLEAR_DIST=0
SET CLEAR_DEPENDENCIES=0
SET CLEAR_ARTIFACTS=0

:loop
if "%~1" NEQ "" (
	IF /i "%~1"=="--all" (
		SET ARGFOUND=1	
		SET CLEAR_DIST=1
		SET CLEAR_DEPENDENCIES=1
		SET CLEAR_ARTIFACTS=1
		goto found:
	)
	IF /i "%~1"=="--local-tmp" (
		SET ARGFOUND=1	
		SET CLEAR_ARTIFACTS=1
		goto found:
	)	
	IF /i "%~1"=="--dist" (
		SET ARGFOUND=1	
		SET CLEAR_DIST=1
		goto found:
	)
	IF /i "%~1"=="--dependencies" (
		SET ARGFOUND=1	
		SET CLEAR_DEPENDENCIES=1
		goto found:
	)

:found
	SHIFT
	goto loop:
)

if "%ARGFOUND%" == "0" (
	echo Usage: win32-clean.bat [--all --local-tmp --dist --dependencies]
	exit /B
)
	
if "%CLEAR_DIST%" == "1" (
    echo Deleting
	rmdir /S ..\dist
)

if "%CLEAR_ARTIFACTS%" == "1" (
    echo Deleting
	rmdir /S ..\local-tmp
)

if "%CLEAR_DEPENDENCIES%" == "1" (
    echo Deleting
	rmdir /S ..\dependencies
)

