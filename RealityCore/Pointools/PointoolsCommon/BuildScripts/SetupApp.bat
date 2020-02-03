@echo off

set PLATFORM=X86
set VS=VS2005

if "%1" == "" (goto USAGE)

if "%1" == "32" (set PLATFORM=X86)
if "%1" == "64" (set PLATFORM=X64)

if "%2" == "VS2005" (set VS=VS2005)
if "%2" == "VS2012" (set VS=VS2012)


if "%PLATFORM%" == "X86" (set OUT_BUILDCONTEXTS=%OUTROOT%Winx86\BuildContexts\)
if "%PLATFORM%" == "X64" (set OUT_BUILDCONTEXTS=%OUTROOT%Winx64\BuildContexts\)

goto DO_COPY


:USAGE

echo.
echo USAGE:
echo.
echo SetupApp ^<32^|64^> ^<VS2005^|VS2012^>
echo.
goto END

:PTAPP_NOT_DEFINED

echo.
echo.
echo Error: Environment Variable PTAPP_32 or PTAPP_64 must be set to Application destination folder
echo.
echo e.g. C:\Program Files\Bentley\MicroStation 08.21.02.079\MicroStation\mdlsys\Asneeded
echo.
echo.

goto END



:DO_COPY

if "%PLATFORM%" == "X86" (set DEST_DIR=%PTAPP_32%)
if "%PLATFORM%" == "X64" (set DEST_DIR=%PTAPP_64%)

if "%DEST_DIR%" == "" (goto PTAPP_NOT_DEFINED)

echo.
echo Setting up application...
echo.
echo Platform                : %PLATFORM%
echo Visual Studio Build     : %VS%
echo Application Directory   : %DEST_DIR%
echo.
echo.
echo PointoolsIO - PODFormats...
xcopy /Y /Q "%OUT_BUILDCONTEXTS%PointoolsIO_PODFormats_%VS%\Delivery\*.dll" "%DEST_DIR%\"
echo.
echo.
echo PointoolsIO - PODWriter...
xcopy /Y /Q "%OUT_BUILDCONTEXTS%PointoolsIO_PODWriter_%VS%\Delivery\*.dll" "%DEST_DIR%\"
echo.
echo.
echo PointoolsVortexAPI...
xcopy /Y /Q "%OUT_BUILDCONTEXTS%PointoolsVortexAPI_%VS%\Delivery\*.dll" "%DEST_DIR%\"
echo.
echo.
echo BatchImporter EXE (Main)...
xcopy /Y /Q "%OUT_BUILDCONTEXTS%PointoolsIO_BatchImporter_%VS%\Delivery\*.exe" "%DEST_DIR%\"
echo.
echo.


:END

