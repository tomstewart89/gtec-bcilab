@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set InitEnvScript=win32-init_env_command.cmd
set PAUSE=pause

REM #######################################################################################

call "!InitEnvScript!"

REM #######################################################################################

set ov_script_dir=%CD%
set ov_build_dir=%ov_script_dir%\..\local-tmp\visual
set ov_install_dir=%ov_script_dir%\..\dist

mkdir %ov_build_dir% 2>NUL
cd /D %ov_build_dir%

echo Generating %VCSMake% project files to %ov_build_dir% ...

cmake %ov_script_dir%\.. -G"%VSCMake%" -DCMAKE_INSTALL_PREFIX=%ov_install_dir%
IF NOT "!ERRORLEVEL!" == "0" goto terminate_error

echo.
echo Visual studio projects generation terminated successfully !
echo.

goto terminate_success

REM #######################################################################################

:terminate_error

echo.
echo An error occured during building process !
echo.
%PAUSE%

goto terminate

REM #######################################################################################

:terminate_success

%PAUSE%

goto terminate

REM #######################################################################################

:terminate

cd %ov_script_dir%



