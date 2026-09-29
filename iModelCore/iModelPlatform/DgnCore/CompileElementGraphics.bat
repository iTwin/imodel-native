ECHO ON
rem SETLOCAL

rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See LICENSE.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

SET BaseName=ElementGraphics
SET SrcDir=%SrcRoot%imodel02\iModelCore\iModelPlatform\DgnCore\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%imodel02\iModelCore\iModelPlatform\PrivateApi\DgnPlatformInternal\DgnCore\
SET OutFile=%OutDir%%BaseName%.fb.h

SET CompileExe=%SrcRoot%imodel02\iModelCore\libsrc\flatbuffers\bin\beflatc.exe
SET CompileCmd=%CompileExe% -c
ECHO Using compiler at '%CompileExe%'

rem ***********************************************************************************************
rem ***********************************************************************************************

IF NOT EXIST %OutDir% MKDIR %OutDir% || (
    ECHO Could not create '%OutDir%'
    GOTO:EOF
    )

rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Compiling '%SrcFile%'...

%CompileCmd% %SrcFile% || (
    ECHO Compile failed
    )

IF NOT EXIST %GeneratedFile% ECHO Failed to generate '%GeneratedFile%' && GOTO:EOF

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Moving '%GeneratedFile% to %OutFile%'...

MOVE %GeneratedFile% %OutFile% > NUL || (
    ECHO Could not move header '%GeneratedFile%' -> '%OutFile%'
    GOTO:EOF
    )

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Successfully created:
ECHO.    '%OutFile%'
