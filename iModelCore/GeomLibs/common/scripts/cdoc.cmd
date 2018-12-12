@echo off

set SOURCEFILE=%1
set TARGETFILE=%2
set TEMPFILE=%TEMP%\cdoc.tmp

if EXIST %SOURCEFILE% goto doit
@echo Usage cdoc pathToSource pathToTarget
goto done

:doit
gema -f %GEMADIR%g\cdoc.g %SOURCEFILE% %TEMPFILE%
copy "%GEMADIR%g\cdoc.head"+"%TEMPFILE%"+"%GEMADIR%g\cdoc.tail" %TARGETFILE%

:done