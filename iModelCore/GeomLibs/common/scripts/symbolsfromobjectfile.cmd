@echo off
if "%GEMADIR%" == "" set GEMADIR=%MSJ%utils\geom\scripts\g\

if "%1" == "" goto error

set inputFile=%1
if EXIST %inputFile% goto doit1
goto error


:doit1

set gemaFile=%GEMADIR%g\symbols.g
if "%2" == "" goto doit2

set gemaFile=%2
if EXIST %gemaFile% goto doit2
set gemaFile=%2.g
if EXIST %gemaFile% goto doit2
set gemaFile=%GEMADIR%g\%2
if EXIST %gemaFile% goto doit2
set gemaFile=%GEMADIR%g\%2.g
if EXIST %gemaFile% goto doit2
goto error


:doit2

dumpbin /symbols %1 | gema -f %gemaFile%

goto done

:error
echo usage: objfilesymbols objectFile.obj

:done