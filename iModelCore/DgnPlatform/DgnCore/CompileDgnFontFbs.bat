@ECHO OFF
SETLOCAL

rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See COPYRIGHT.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

SET BaseName=DgnFont
SET SrcDir=%SrcRoot%DgnPlatform\DgnCore\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%DgnPlatform\PublicApi\DgnPlatform\
SET OutFile=%OutDir%%BaseName%.fb.h

REM flatbuffers comes from nuget now... just use the latest we can find:
for /f %%i IN ('dir "%SrcRoot%nuget\FlatBuffersNuget_x64*" /ad /b /on') DO (
set latestFBNugetDir=%SrcRoot%nuget\%%i
)

SET CompileExe=%latestFBNugetDir%\native\bin\beflatc.exe
SET CompileCmd=%CompileExe% -c
ECHO Using compiler at '%CompileExe%'

rem ***********************************************************************************************
rem ***********************************************************************************************

IF NOT EXIST %OutDir% MKDIR %OutDir% || (
    ECHO Could not create '%OutDir%'
    EXIT 1
    )

rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Compiling '%SrcFile%'...

%CompileCmd% %SrcFile% || (
    ECHO Compile failed
    EXIT 1
    )

IF NOT EXIST %GeneratedFile% ECHO Failed to generate '%GeneratedFile%' && EXIT 1

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Moving '%GeneratedFile% to %OutFile%'...

ECHO //__BENTLEY_INTERNAL_ONLY__ > %OutFile% || (
    ECHO Could not create output file '%OutFile%'
    EXIT 1
    )

TYPE %GeneratedFile% >> %OutFile% || (
    ECHO Could not create output file '%OutFile%'
    EXIT 1
    )

DEL %GeneratedFile% || (
    ECHO Could not delete temporary file '%GeneratedFile%'
    EXIT 1
    )

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Successfully created:
ECHO.    '%OutFile%'
