@echo off

set macrofile=%1
if EXIST %macrofile% goto checkDataFile

set macrofile=%GEMADIR%\%1
if EXIST %macrofile% goto checkDataFile

set macrofile=%GEMADIR%\g\%1
if EXIST %macrofile% goto checkDataFile

goto fileError

:checkDataFile

set datafile=%2
if EXIST %datafile% goto doIt

:fileError
@echo Usage: gemaproc gemafile datafile
@echo You requested gemafile %macrofile%
@echo You requested datafile %datafile%
@echo Be sure that both exist.
goto done


:doIt

gema -f %macrofile% %datafile% %TEMP%\gemawork.
cmp -s %datafile% %TEMP%\gemawork.
if %ERRORLEVEL% == 0 goto nochange

echo %datafile% (changed) >> gemaproc.log
echo %datafile% changed
copy %TEMP%\gemawork. %datafile%
goto done

:nochange
echo %datafile% no change >> gemaproc.log
:done
@echo on
