@ECHO OFF

REM This script is intended to be called via the launch script setting the environment properly

set Extension=%~x1
REM remove the dot before the extension
set Extension=%Extension:~1%
set DestinationExtension=%~x2
set DestinationExtension=%DestinationExtension:~1%

setlocal EnableDelayedExpansion

set FormatFrom=(dat vhdr csv gdf ov)
set FormatTo=(csv gdf edf ov)

rem count arguments
set argC=0
for %%x in (%*) do Set /A argC+=1

if not %argC% == 2 (
  echo Usage: openvibe-convert [sourceFile] [destinationFile]
  echo.
  echo WARNING: This script is provided as an example only. There is no guarantee that the files 
  echo will be correctly converted or that they retain all information across the different formats. 
  echo.
  echo Converts stream files from source to dest.
  echo File formats are discovered from the filename extensions.
  echo.
  echo Supported sources are "%FormatFrom%"
  echo Supported destinations are "%FormatTo%"
  echo.
  pause
  GOTO:EOF
)

if %Extension%==~1 (
  echo File extension not found from source filename "%1".
  pause
  GOTO:EOF
)

if %DestinationExtension%==~1 (
  echo File extension not found from destination filename "%2".
  pause
  GOTO:EOF
)

set From=false
set /a i=0

set From=false
for %%x in %FormatFrom% do (
  if %%x==%Extension% (
    set From=true
  )
)

set To=false
for %%x in %FormatTo% do (
  if %%x==%DestinationExtension% (
    set To=true
  )
)

if %From%==false (
  echo 'There is no reader box for the .%Extension% format in openvibe' 
  goto:EOF
)

if %To%==false (
  echo 'There is no writer box for the .%DestinationExtension% format in openvibe' 
  goto:EOF
)

set "ScenarioFolder=%OV_PATH_DATA%\scenarios\convert\"

set "ScenarioToOpen=%Extension%2%DestinationExtension%.xml"

set OV_CONVERT_SRC=%1
set OV_CONVERT_DEST=%2

REM launch designer
REM echo %OV_PATH_BIN%\openvibe-designer --no-gui --no-session-management --play-fast !ScenarioFolder!!ScenarioToOpen!
%OV_PATH_BIN%\openvibe-designer --no-gui --no-session-management --play-fast !ScenarioFolder!!ScenarioToOpen!

endlocal

