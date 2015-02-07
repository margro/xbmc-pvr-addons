@ECHO OFF
REM This batch file copies the PVR addon to the Kodi addons folder when building the
REM addons inside the Kodi source tree

REM Usage:
REM Add this script as Post-Build event to the pvr addon project
REM Example Post-Build command line: "$(ProjectDir)\..\..\..\..\scripts\copy_pvr_addon.bat" "$(TargetDir)" "$(ProjectName)"
REM 1st argument should be the source directory of the addon (where the *.pvr file is located)
REM 2nd argument should be the wanted pvr addon name (destination folder name) e.g. "pvr.demo"
REM This script should be executed in the project folder of the addon
REM e.g. addons\pvr.demo\project\VS201xExpress

REM echo Current directory: %CD%
SET CUR_DIR=%CD%

SET KODI_ADDONS_DIR=..\..\..\..\..\addons

REM We have found the Kodi addons folder when it contains 'library.xbmc.pvr'
IF NOT EXIST "%KODI_ADDONS_DIR%\library.xbmc.pvr" (
  ECHO Not building inside the Kodi source tree. Skipped PVR addon copy to the Kodi addons folder.
  EXIT
)
CD "%KODI_ADDONS_DIR%"

SET ADDONDSTDIR=%CD%
SET ADDONSRCDIR=%1
REM remove the double quotes:
SET ADDONSRCDIR=%ADDONSRCDIR:"=%

REM remove the trailing \ otherwise xcopy will fail
IF %ADDONSRCDIR:~-1%==\ SET ADDONSRCDIR=%ADDONSRCDIR:~0,-1%

ECHO Copying the %2 addon from %ADDONSRCDIR% to the Kodi addons folder %ADDONDSTDIR%
robocopy "%ADDONSRCDIR%" "%ADDONDSTDIR%\%2" *.* /MIR /XF *.ilk *.exp *.lib *.xml.in /NP
EXIT 0
