@ECHO OFF
REM This batch file copies the PVR addon to the XBMC addons folder when building the
REM addons inside the XBMC source tree

REM Usage:
REM Add this script as Post-Build event to the pvr addon project
REM Example Post-Build command line: "$(ProjectDir)\..\..\..\..\scripts\copy_pvr_addon.bat" "$(TargetDir)" "$(ProjectName)"
REM 1st argument should be the source directory of the addon (where the *.pvr file is located)
REM 2nd argument should be the wanted pvr addon name (destination folder name) e.g. "pvr.demo"
REM This script should be executed in the project folder of the addon
REM e.g. addons\pvr.demo\project\VS2010Express

REM echo Current directory: %CD%
SET CUR_DIR=%CD%

SET XBMC_ADDONS_DIR=..\..\..\..\..\addons

REM We have found the XBMC addons folder if it contains the xbmc.pvr library
IF NOT EXIST "%XBMC_ADDONS_DIR%\library.xbmc.pvr"  EXIT

CD "%XBMC_ADDONS_DIR%"
REM ECHO Found the XBMC addons folder at: %CD%

SET ADDONDSTDIR=%CD%
SET ADDONSRCDIR=%1
REM remove the double quotes:
SET ADDONSRCDIR=%ADDONSRCDIR:"=%
REM remove the trailing \ otherwise xcopy will fail
IF %ADDONSRCDIR:~-1%==\ SET ADDONSRCDIR=%ADDONSRCDIR:~0,-1%

IF EXIST %2 (
  ECHO Found an existing version of the pvr addon %2 in the XBMC addons folder %ADDONDSTDIR%. Deleting this version...
  RMDIR /S /Q %2
)

ECHO Copying the %2 addon from %ADDONSRCDIR% to the XBMC addons folder %ADDONDSTDIR%
REM exclude some intermediate VS2010 files
ECHO .ilk>exclude.txt
ECHO .exp>>exclude.txt
ECHO .lib>>exclude.txt
XCOPY "%ADDONSRCDIR%" "%ADDONDSTDIR%\%2\" /Y /R /I /E /Q /EXCLUDE:exclude.txt
DEL exclude.txt
