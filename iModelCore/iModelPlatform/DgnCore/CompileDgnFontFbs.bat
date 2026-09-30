@ECHO OFF
SETLOCAL

rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See LICENSE.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

SET BaseName=DgnFont
SET SrcDir=%SrcRoot%imodel02\iModelCore\iModelPlatform\DgnCore\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%imodel02\iModelCore\iModelPlatform\PublicApi\DgnPlatform\
SET OutFile=%OutDir%%BaseName%.fb.h

SET CompileExe=%SrcRoot%imodel02\iModelCore\libsrc\flatbuffers\bin\beflatc.exe
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
    )

IF NOT EXIST %GeneratedFile% ECHO Failed to generate '%GeneratedFile%' && EXIT 1

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Moving '%GeneratedFile% to %OutFile%'...

TYPE %GeneratedFile% > %OutFile% || (
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
