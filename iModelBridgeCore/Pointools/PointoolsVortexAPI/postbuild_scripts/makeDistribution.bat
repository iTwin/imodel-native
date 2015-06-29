@echo off

setlocal enabledelayedexpansion

SET thisdir=%CD%

@REM allow custom distributions for different Vortex users:
@REM normal distribution: 	customDistrib=0
@REM Siemens distribution: 	customDistrib=1
@REM FME distribution: 		customDistrib=2
SET customDistrib=0

@REM path for copying PRG LKG Vortex and PODFormats/PODwriter DLLS/libs from if required
@REM (currently used when making the Siemens or FME distribution)
@REM an example command line for building a custom build using the LKG is:
@REM makeDistribution.bat -distrib siemens -uselkg 2-1-0-1
SET useLKG=0
SET lkgVersion="0-0-0-0"

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
		SET customDistrib=1	
	)
	if "%2"=="fme" (
		echo ******************************************************************
		echo Building custom distribution for FME
				
		SET customDistrib=2	
	)
	if NOT "%2%"=="siemens" (
		if NOT "%2%"=="fme" (	
			echo ******************************************************************
			echo User not recognized, the only supported distributions are 
			echo "siemens" or "fme"			
			goto END_ERROR
		)
	)	
	if "%3%"=="-uselkg" (		
		SET useLKG=1
		@REM the lkg version should be the fourth param, e.g. 2-1-0-1, this refers to the folder name
		@REM of the build at \\builds\prgbuilds-readonly\LKGOutput\PointoolsVortexAPI\
		SET lkgVersion=%4%
		)				
)

SET PRG_LKG_Path=\\builds\prgbuilds-readonly\LKGOutput\PointoolsVortexAPI\%lkgVersion%

@REM setup dll/lib paths depending on if we're copying the from a local build or LKG
SET vortexDLL86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexDLL64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexLib86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexLib64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012
SET podWriterDLL86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODWriter_VS2012
SET podWriterDLL64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODWriter_VS2012
SET podFormatsDLL86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODFormats_VS2012
SET podFormatsDLL64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODFormats_VS2012
SET podFormatsLib86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsIO_PODFormats_VS2012
SET podFormatsLib64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsIO_PODFormats_VS2012
SET vortexFEDLL86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexFEDLL64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexFELib86Path=%BuildOutDir%\Winx86\Build\Pointools\PointoolsVortexAPI_VS2012
SET vortexFELib64Path=%BuildOutDir%\Winx64\Build\Pointools\PointoolsVortexAPI_VS2012
if %useLKG%==0 (
	echo ***  Using locally built Vortex and POD DLLs  ***	
)
if %useLKG%==1 (
	echo ***  Using Vortex and POD DLLs from PRG LKG build ***
	SET vortexDLL86Path=%PRG_LKG_Path%\Winx86\PointoolsVortexAPI_VS2012\Delivery
	SET vortexDLL64Path=%PRG_LKG_Path%\Winx64\PointoolsVortexAPI_VS2012\Delivery
	SET vortexLib86Path=%PRG_LKG_Path%\Winx86\PointoolsVortexAPI_VS2012\Delivery
	SET vortexLib64Path=%PRG_LKG_Path%\Winx64\PointoolsVortexAPI_VS2012\Delivery
	SET podWriterDLL86Path=%PRG_LKG_Path%\Winx86\PointoolsIO_PODWriter_VS2012\Delivery
	SET podWriterDLL64Path=%PRG_LKG_Path%\Winx64\PointoolsIO_PODWriter_VS2012\Delivery
	SET podFormatsDLL86Path=%PRG_LKG_Path%\Winx86\PointoolsIO_PODFormats_VS2012\Delivery
	SET podFormatsDLL64Path=%PRG_LKG_Path%\Winx64\PointoolsIO_PODFormats_VS2012\Delivery
	SET podFormatsLib86Path=%PRG_LKG_Path%\Winx86\PointoolsIO_PODFormats_VS2012\Delivery
	SET podFormatsLib64Path=%PRG_LKG_Path%\Winx64\PointoolsIO_PODFormats_VS2012\Delivery
	SET vortexFEDLL86Path=%PRG_LKG_Path%\Winx86\PointoolsVortexAPI_VS2012\Delivery
	SET vortexFEDLL64Path=%PRG_LKG_Path%\Winx64\PointoolsVortexAPI_VS2012\Delivery
	SET vortexFELib86Path=%PRG_LKG_Path%\Winx86\PointoolsVortexAPI_VS2012\Delivery
	SET vortexFELib64Path=%PRG_LKG_Path%\Winx64\PointoolsVortexAPI_VS2012\Delivery
)	

echo ******************************************************************

call %Pointools%\PointoolsIO\postbuild_scripts\makeWorkingBuild.bat
call %Pointools%\PointoolsIO\postbuild_scripts\makeDepsFolder.bat

cd %thisdir%

@REM get date in YYMMDD format, WARNING: might be local dependent
set compile_date=%date:~8,2%%date:~3,2%%date:~0,2%

SET version=2.0.0.215
SET topfolder=%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%
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

robocopy "%vortexDLL86Path%" 	"%folder%\distrib\vc8" 		PointoolsVortexAPI.dll
robocopy "%vortexDLL64Path%" 	"%folder%\distrib\vc11" 	PointoolsVortexAPI.dll
robocopy "%vortexLib86Path%" 	"%folder%\lib\vc8"			PointoolsVortexAPI.lib
robocopy "%vortexLib64Path%" 	"%folder%\lib\vc11" 		PointoolsVortexAPI.lib

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
robocopy "%vortexDLL86Path%"  	 	"%folder%\examples\bin\x86"		PointoolsVortexAPI.dll
robocopy "%vortexDLL64Path%"   		"%folder%\examples\bin\x64" 	PointoolsVortexAPI.dll
robocopy "%podWriterDLL86Path%"  	"%folder%\examples\bin\x86"		PODwriter.dll
robocopy "%podWriterDLL64Path%" 	"%folder%\examples\bin\x64"		PODwriter.dll
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

robocopy "%podWriterDLL86Path%"		"%pod%\bin\x86"		PODWriter.dll
robocopy "%podWriterDLL64Path%"		"%pod%\bin\x64"		PODWriter.dll

@REM PODFormats(but not for FME distribution)
@REM --------------------------------------------------------------------------------------
if NOT %customDistrib%==2 (
	echo ******************************************************************	
	echo Including PODFormats
	echo ******************************************************************
	robocopy "%podFormatsDLL86Path%"		"%pod%\bin\x86"		PODFormats.dll
	robocopy "%podFormatsDLL64Path%"		"%pod%\bin\x64"		PODFormats.dll	
	robocopy "%podFormatsLib86Path%"		"%pod%\lib\x86"		PODFormats.lib
	robocopy "%podFormatsLib64Path%"		"%pod%\lib\x64"		PODFormats.lib
)

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
				
@REM Copy in 3rd parties dependencies for POD Formats (but not for FME distribution)
@REM --------------------------------------------------------------------------------------
if NOT %customDistrib%==2 (
	echo ******************************************************************
	echo Including all PODFormats dependancies
	echo ******************************************************************
	
	xcopy "%Pointools%\PointoolsIO\deps\x86\*.dll"			"%pod%\deps\x86" /Y
	xcopy "%Pointools%\PointoolsIO\deps\x64\*.dll"			"%pod%\deps\x64" /Y
)


@REM Setup Extensions
@REM --------------------------------------------------------------------------------------
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\VortexFeatureExtract.h"		"%extensions%\include\"
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\VortexFeatureExtractAPI.h"	"%extensions%\include\"
copy "%PointoolsVortex%\extensions\VortexFeatureExtract\GeomTypes.h"				"%extensions%\include\"

robocopy "%vortexFELib86Path%"		"%extensions%\lib\vc8"		VortexFeatureExtract.lib
robocopy "%vortexFELib64Path%"		"%extensions%\lib\vc11"		VortexFeatureExtract.lib
robocopy "%vortexFEDLL86Path%"		"%extensions%\bin\vc8"		VortexFeatureExtract.dll
robocopy "%vortexFEDLL64Path%"		"%extensions%\bin\vc11"		VortexFeatureExtract.dll

@REM add Bentley SELECT licensing DLLs to distribution (but not for Siemens distribution)
@REM --------------------------------------------------------------------------------------
if NOT %customDistrib%==1 (
	echo ******************************************************************
	echo Including SELECT licensing files
	echo ******************************************************************
	
	mkdir "%folder%\deps"
	mkdir "%folder%\deps\x86"
	mkdir "%folder%\deps\x64"

	xcopy "%BuildOutDir%\Winx86\BuildContexts\bsibin\LicenseOut\bin\*.exe"	"%folder%\deps\x86" /Y
	xcopy "%BuildOutDir%\Winx86\BuildContexts\bsibin\LicenseOut\bin\*.dll"	"%folder%\deps\x86" /Y

	xcopy "%BuildOutDir%\Winx64\BuildContexts\bsibin\LicenseOut\bin\*.exe"	"%folder%\deps\x64" /Y
	xcopy "%BuildOutDir%\Winx64\BuildContexts\bsibin\LicenseOut\bin\*.dll"	"%folder%\deps\x64" /Y
	
	@REM also copy to examples bin folder 
	xcopy "%BuildOutDir%\Winx86\BuildContexts\bsibin\LicenseOut\bin\*.exe"	"%folder%\examples\bin\x86" /Y
	xcopy "%BuildOutDir%\Winx86\BuildContexts\bsibin\LicenseOut\bin\*.dll"	"%folder%\examples\bin\x86" /Y

	xcopy "%BuildOutDir%\Winx64\BuildContexts\bsibin\LicenseOut\bin\*.exe"	"%folder%\examples\bin\x64" /Y
	xcopy "%BuildOutDir%\Winx64\BuildContexts\bsibin\LicenseOut\bin\*.dll"	"%folder%\examples\bin\x64" /Y
)

@REM remove source control files
@REM --------------------------------------------------------------------------------------
cd "%PointoolsVortex%\Distrib\"
del ".MySCMServerInfo" /Q /S

@REM --------------------------------------------------------------------------------------
cd %PointoolsVortex%\distrib

@REM Copy matching release notes into distribution
copy %releasenotes% "PointoolsVortexAPI-%version%\"

@REM EULA and license notice
copy %PointoolsVortex%\notices.txt 	"%topfolder%\notices.txt"
copy %PointoolsVortex%\EULA.txt 	"%topfolder%\EULA.txt"

@REM --------------------------------------------------------------------------------------
@REM Remove clash and clientserver if this is a distribution for Siemens or FME and copy
@REM built DLLs from the PRG LKG rather than useing locally built ones
@REM Remove metadata examples as well from all external distributions as the Smart Property 
@REM Grid is not licensed for distribution externally
cd %PointoolsVortex%\postbuild_scripts
SET removeClashAndCS=0
if %customDistrib%==1 (
	SET removeClashAndCS=1
)	
if %customDistrib%==2 (
	SET removeClashAndCS=1
)	
if NOT %customDistrib%==0 (
	echo ******************************************************************
	echo removing Clash, ClientServer and MetaData examples
	echo ******************************************************************

	echo ON
	perl removeFromDistribution.plx distribDir="%PointoolsVortex%\Distrib\PointoolsVortexAPI-%version%" removeClash=%removeClashAndCS% removeClientServer=%removeClashAndCS% removeMetaData=1 removePDBs=1
	@echo OFF
	
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

@REM reminder about where the DLLs used in this distribution came from
@REM to use LKGs do: makedistribution.bat -distrib fme uselkg
@REM (or replace fme with siemens for Siemens distrib)
echo ******************************************************************
if %useLKG%==0 (
	echo ***  Using locally built Vortex and POD DLLs  ***
)
if %useLKG%==1 (
	echo ***  Using Vortex and POD DLLs from PRG LKG build ***
)
echo ******************************************************************

endlocal
