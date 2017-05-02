rem @ECHO OFF

rem ------------------------------------------------------------------------------------
rem      $Source: serialization/src/FlatBuffer/CompileFbs.bat $
rem   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
rem ------------------------------------------------------------------------------------

SET BaseName=allcg
SET SrcDir=%SrcRoot%GeomLibs\Serialization\src\Flatbuffer\
SET SrcFile=%SrcDir%%BaseName%.flatbuf
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%GeomLibs\Serialization\Flatbuffer\
SET OutFile=%OutDir%%BaseName%.fb.h
SET TempFile=%OutDir%%BaseName%.fbs
SET OutNetDir=%SrcRoot%BentleyGeometryNet\src\FlatBuffers\gensrc\

SET CompileCmd=%SrcRoot%libsrc\flatbuffers\bin\beflatc.exe -c
SET CompileNETCmd=%SrcRoot%libsrc\flatbuffers\bin\beflatc.exe -n -o %OutNetDir%

SET CompileNETCmd=%SrcRoot%libsrc\flatbuffers\bin\beflatc.exe -n -o %OutNetDir%
SET libsrcFBCSDir=%SrcRoot%libsrc\flatbuffers\source\net\FlatBuffers\
SET BGNetFBCSDir=%SrcRoot%BentleyGeometryNet\src\FlatBuffers\srcFromLibsrcFlatbuffers\


rem ***********************************************************************************************
rem ***********************************************************************************************

IF NOT EXIST %OutDir% MKDIR %OutDir% || (
    ECHO Could not create '%OutDir%'
    goto done
    )

IF NOT EXIST %OutNetDir% (
    ECHO Could not find %OutNetDir%
    goto done
    )

IF NOT EXIST %BGNetFBCSDir% (
    ECHO Could not find %BGNetFBCSDir%
    goto done
    )

if .%1 EQU .debug (
    echo libsrcFBCSDir = %libsrcFBCSDir%
    echo BGNetFBCSDir = %BGNetFBCSDir%
    )

rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO namespace Bentley.Geometry.FB; > %TempFile%
TYPE %SrcFile% >> %TempFile%

rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Compiling '%TempFile%'...

%CompileCmd% %TempFile% || (
    rem DEL %TempFile%
    ECHO Compile failed
    goto done
    )

DEL %TempFile%
IF NOT EXIST %GeneratedFile% ECHO Failed to generate '%GeneratedFile%' && goto done

ECHO Done Generating %GeneratedFile%
ECHO.
ECHO.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO namespace Bentley.GeometryNET.FB; > %TempFile%
TYPE %SrcFile% >> %TempFile%

rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Compiling '%TempFile%'...

%CompileNETCmd% %TempFile% || (
    DEL %TempFile%
    ECHO Compile failed
    goto done
    )
DEL %TempFile%

rem copy cs source files for flatbuffer table/struct abstractions into BGNet ....(BUT THIS MAY OVERWRITE LOCAL CHANGES -- e.g. BlockCopy fixes)
rem COPY %libsrcFBCSDir%*.cs %BGNetFBCSDir%

MOVE %OutNetDir%Bentley\GeometryNET\FB\* %OutNetDir% >nul
RD /S /Q %OutNetDir%Bentley
ECHO Done creating classes in %OutNetDir%
ECHO.

:done