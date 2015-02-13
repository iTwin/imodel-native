@echo off

SET thisdir=%CD%

@REM allow custom distributions for different Vortex users:
@REM normal distribution: customDistrib=0
@REM Siemens distribution: customDistrib=1
SET customDistrib=0


@REM Vortex Paths - override environment variable settings
SET Pointools=D:\PointoolsSiemens\src\Pointools
echo overriding Pointools environment var to be %Pointools%
SET PointoolsVortex=%Pointools%\PointoolsVortexAPI
echo overriding PointoolsVortex environment var to be %Pointools%\PointoolsVortexAPI
SET BuildOutDir=D:\PointoolsSiemens\out
echo setting BuildOutDir var to be %BuildOutDir%


if "%1"=="-distrib" (
	if "%2"=="siemens" (	
		echo ******************************************************************
		echo Building custom distribution for Siemens
		echo ******************************************************************
		SET customDistrib=1	
	)
	if NOT "%2"=="siemens" (	
		echo ******************************************************************
		echo User not recognized, the only supported custom distribution is "siemens"
		echo ******************************************************************
		goto END_ERROR
	)
)

call %Pointools%\PointoolsIO\postbuild_scripts\makeWorkingBuild.bat
call %Pointools%\PointoolsIO\postbuild_scripts\makeDepsFolder.bat

cd %thisdir%

@REM get date in YYMMDD format, WARNING: might be local dependent
set compile_date=%date:~8,2%%date:~3,2%%date:~0,2%

SET version=2.0.0.204
SET folder=%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%\VortexAPI
SET releasenotes=%PointoolsVortex%\ReleaseNotes\PointoolsVortexAPI-ReleaseNotes-%version%.txt
SET pod=%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%\PointoolsIO
SET extensions=%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%\VortexExtensions

if exist %releasenotes% goto FOUND_RELEASE_NOTES

set PTBuildError="Matching release notes not found"
set PTBuildErrorInfo=%releasenotes%
goto END_ERROR

:FOUND_RELEASE_NOTES


@REM Version stamp original in place binaries
@REM --------------------------------------------------------------------------------------
@REM these are stamped by the build system now
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODWriter.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODWriter.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.dll
@REM %POINTOOLSBUILDTOOLS%\stampver.exe -f%version% -p%version% %BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.dll

mkdir "%folder%\include"
mkdir "%folder%\src"
mkdir "%folder%\examples"
mkdir "%folder%\doc"
mkdir "%folder%\distrib"
mkdir "%folder%\distrib\vc8"
mkdir "%folder%\distrib\vc11"
mkdir "%folder%\lib"
mkdir "%folder%\lib\vc8"
mkdir "%folder%\lib\vc11"
@REM mkdir "%folder%\pdb"
@REM mkdir "%folder%\pdb\vc8"
@REM mkdir "%folder%\pdb\vc11"

mkdir "%extensions%\bin\vc8"
mkdir "%extensions%\bin\vc11"
mkdir "%extensions%\lib\vc8"
mkdir "%extensions%\lib\vc11"
mkdir "%extensions%\include"

@REM Set up the API files
@REM --------------------------------------------------------------------------------------
copy "%PointoolsVortex%\include\ptapi\PointoolsVortexAPI_import.h" 	"%folder%\include"
copy "%PointoolsVortex%\include\ptapi\PointoolsVortexAPI_ResultCodes.h" "%folder%\include"
copy "%PointoolsVortex%\include\vortexobjects\VortexObjects_import.h" "%folder%\include"
copy "%PointoolsVortex%\include\vortexobjects\IClashObjectManager.h" "%folder%\include"
copy "%PointoolsVortex%\include\vortexobjects\IClashObject.h" "%folder%\include"
copy "%PointoolsVortex%\include\vortexobjects\IClashTree.h" "%folder%\include"
copy "%PointoolsVortex%\include\vortexobjects\IClashNode.h" "%folder%\include"

copy "%PointoolsVortex%\src\ptapi\PointoolsVortexAPI_import.cpp"	"%folder%\src"

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll" 		"%folder%\distrib\vc8"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll" 		"%folder%\distrib\vc11"
copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.lib" 		"%folder%\lib\vc8"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.lib" 		"%folder%\lib\vc11"

@REM copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.pdb" 		"%folder%\pdb\vc8\PointoolsVortexAPI.pdb"
@REM copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.pdb" 		"%folder%\pdb\vc11\PointoolsVortexAPI.pdb"
@REM copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.pdb" 		"%folder%\pdb\vc8\VortexFeatureExtract.pdb"
@REM copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.pdb" 		"%folder%\pdb\vc11\VortexFeatureExtract.pdb"


@REM Set up the examples - do not copy exe, user must build
@REM --------------------------------------------------------------------------------------
xcopy "%PointoolsVortex%\examples\*.vcproj" 			"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.cpp" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.h" 					"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.sln" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.rc" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.rc2" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.ico" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.bmp" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.png" 				"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.vcxproj" 			"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\*.vcxproj.filters" 	"%folder%\examples" /S /Y
xcopy "%PointoolsVortex%\examples\metadata\PropertyGrid\bin\*.lib" 		"%folder%\examples\metadata\PropertyGrid\bin\*.lib" /S /Y

mkdir "%folder%\examples\lic"
mkdir "%folder%\examples\bin\x86"
mkdir "%folder%\examples\bin\x64"

@REM copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.pdb" 		"%folder%\examples\bin\x86\PointoolsVortexAPI.pdb"
@REM copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.pdb" 		"%folder%\examples\bin\x64\PointoolsVortexAPI.pdb"

@REM copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.pdb" 		"%folder%\examples\bin\x86\VortexFeatureExtract.pdb"
@REM copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.pdb" 		"%folder%\examples\bin\x64\VortexFeatureExtract.pdb"


@REM Delete stuff that should not be distributed
@REM --------------------------------------------------------------------------------------
del 	"%folder%\examples\shadow-maps\*.*"		/S /Q
rmdir	"%folder%\examples\shadow-maps" 		/S /Q
del 	"%folder%\examples\dev-examples.sln"		/Q
del 	"%folder%\examples\bin\test*.exe" 		/Q
rmdir 	"%folder%\examples\editing\testing"		/S /Q
rmdir 	"%folder%\examples\inc"				/S /Q
del 	"%folder%\examples\*vc9*" 			/S /Q

echo "contact vortex@pointools.com for your license file" >> "%folder%\examples\lic\place your vortexLicense.c file here"


@REM Dlls for examples
@REM --------------------------------------------------------------------------------------
copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll"   	"%folder%\examples\bin\x86" 
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\PointoolsVortexAPI.dll"   	"%folder%\examples\bin\x64" 
copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODwriter.dll"  		"%folder%\examples\bin\x86"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODwriter.dll" 			"%folder%\examples\bin\x64"
copy "%PointoolsLib%\gl\bin\glut32.dll"  														"%folder%\examples\bin\x86"
copy "%Pointools%\PointoolsLibs\freeglut-2.8.1\lib\VS2012\x64\Release\freeglut.dll"  			"%folder%\examples\bin\x64"
copy "%Pointools%\PointoolsLibs\freeglut-2.8.1\lib\VS2012\x64\Debug\freeglutd.dll"  			"%folder%\examples\bin\x64"
mkdir "%folder%\examples\include\gl"
xcopy "%PointoolsLib%\gl\lib\glu*.lib" 															"%folder%\examples\lib\*.lib" /S /Y
copy "%Pointools%\PointoolsLibs\glui-2.35\src\VS2012\lib\glui64.lib"  							"%folder%\examples\lib"
copy "%Pointools%\PointoolsLibs\glui-2.35\src\VS2012\lib\glui64d.lib"  							"%folder%\examples\lib"
copy "%Pointools%\PointoolsLibs\freeglut-2.8.1\lib\VS2012\x64\Release\freeglut.lib"  			"%folder%\examples\lib"
copy "%Pointools%\PointoolsLibs\freeglut-2.8.1\lib\VS2012\x64\Debug\freeglutd.lib" 			"%folder%\examples\lib"
copy "%PointoolsLib%\gl\glui.h"																	"%folder%\examples\include\gl" 
copy "%PointoolsLib%\gl\glut.h"																	"%folder%\examples\include\gl" 

@REM Set up docs
@REM --------------------------------------------------------------------------------------
copy "%PointoolsVortex%\doc\*.pdf" 								"%folder%\doc\*.pdf"

@REM Setup PointoolsIO
@REM --------------------------------------------------------------------------------------
mkdir "%pod%\include"
mkdir "%pod%\bin\x64"
mkdir "%pod%\bin\x86"
mkdir "%pod%\deps\x64"
mkdir "%pod%\deps\x86"
mkdir "%pod%\examples"
mkdir "%pod%\lib\x64"
mkdir "%pod%\lib\x86"
mkdir "%pod%\doc"
mkdir "%pod%\src"
@REM mkdir "%pod%\pdb"
mkdir "%pod%\bindings\java"
mkdir "%pod%\bindings\csharp"

SET podbuild=%Pointools%\PointoolsIO\workingbuild
copy "%podbuild%\include\*.h" 				"%pod%\include\*.h" 

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODWriter.dll"		"%pod%\bin\x86\PODWriter.dll"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODWriter_VS2012\PODWriter.dll"		"%pod%\bin\x64\PODWriter.dll"

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.dll"		"%pod%\bin\x86\PODFormats.dll"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.dll"		"%pod%\bin\x64\PODFormats.dll"

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.lib"		"%pod%\lib\x86\PODFormats.lib"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODFormats_VS2012\PODFormats.lib"		"%pod%\lib\x64\PODFormats.lib"

@REM copy %Pointools%\PointoolsIO\workingbuild\bin\PODWriter.pdb		%pod%\pdb\"
@REM copy %Pointools%\PointoolsIO\workingbuild\bin\PODFormats.pdb		%pod%\pdb\"
@REM copy %Pointools%\PointoolsIO\workingbuild\bin\PODWriter64.pdb		%pod%\pdb\"
@REM copy %Pointools%\PointoolsIO\workingbuild\bin\PODFormats64.pdb		%pod%\pdb\"


@REM copy "%podbuild%\bin\*-distrib.pdb"		"%pod%\pdb"

copy "%podbuild%\src\*.cpp"					"%pod%\src\*.cpp"
copy "%Pointools%\PointoolsIO\doc\*.pdf"	"%pod%\doc\*.pdf"
copy "%Pointools%\PointoolsIO\src\*.cpp"	"%pod%\src\*.cpp"
xcopy "%podbuild%\examples\*.vcproj"		"%pod%\examples" /S /Y
xcopy "%podbuild%\examples\*.h"				"%pod%\examples" /S /Y
xcopy "%podbuild%\examples\*.cpp"			"%pod%\examples" /S /Y
copy "%podbuild%\java\*.java"				"%pod%\bindings\java\"
copy "%podbuild%\csharp\*.cs"				"%pod%\bindings\csharp\"
				
@REM Copy in 3rd parties dependencies for POD Formats
@REM --------------------------------------------------------------------------------------
xcopy "%Pointools%\PointoolsIO\deps\x86\*.dll"			"%pod%\deps\x86" /Y
xcopy "%Pointools%\PointoolsIO\deps\x64\*.dll"			"%pod%\deps\x64" /Y


@REM Setup Extensions
@REM --------------------------------------------------------------------------------------
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\VortexFeatureExtract.h"		"%extensions%\include\"
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\VortexFeatureExtractAPI.h"	"%extensions%\include\"
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\GeomTypes.h"				"%extensions%\include\"

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.lib"	"%extensions%\lib\vc8\"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.lib"	"%extensions%\lib\vc11\"

copy "%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.dll"	"%extensions%\bin\vc8\"
copy "%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012\VortexFeatureExtract.dll"	"%extensions%\bin\vc11\"

@REM remove source control files
@REM --------------------------------------------------------------------------------------
cd "%PointoolsVortex%\Distrib\"
del ".MySCMServerInfo" /Q /S

@REM --------------------------------------------------------------------------------------
cd %PointoolsVortex%\distrib

@REM Copy matching release notes into distribution
copy %releasenotes% "PointoolsVortexAPI-%version%\"

@REM --------------------------------------------------------------------------------------S
@REM Remove clash and clientserver if this is a distribution for Siemens
cd %PointoolsVortex%\postbuild_scripts
if %customDistrib%==1 (
	echo ******************************************************************
	echo removing Clash and ClientServer examples
	echo ******************************************************************

	perl removeFromDistribution.plx distribDir="%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%" removeClash=1 removeClientServer=1 removePDBs=1
	
	if ERRORLEVEL 1 goto END_ERROR

@REM	copy setupExamplesPreBuild_distrib.bat "%folder%\examples\"
)

@REM --------------------------------------------------------------------------------------
@REM Build PointoolsIO and Pointools PODWriter documentation using Doxygen
cd %POINTOOLS%\Doxygen
call Build_Docs.bat
cd %POINTOOLSVORTEX%\Distrib
rmdir /S /Q "PointoolsVortexAPI-%version%\PointoolsIO\doc\html"
mkdir "PointoolsVortexAPI-%version%\PointoolsIO\doc\html"
xcopy %POINTOOLS%\Doxygen\Docs_IO\html  "PointoolsVortexAPI-%version%\PointoolsIO\doc\html" /S

%POINTOOLSBUILDTOOLS%\zip.exe -e -r -9 "PointoolsVortexAPI-%version%.zip"  "PointoolsVortexAPI-%version%\*.*" 



goto END



:END_ERROR

echo.
echo.
echo "******************************************************************"
echo "Error"
echo %PTBuildError%
echo %PTBuildErrorInfo%
echo "******************************************************************"
echo.

:END
