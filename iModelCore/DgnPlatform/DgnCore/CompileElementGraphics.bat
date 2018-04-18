ECHO ON
rem SETLOCAL

rem ------------------------------------------------------------------------------------
rem      $Source: DgnCore/CompileElementGraphics.bat $
rem   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
rem ------------------------------------------------------------------------------------

SET BaseName=ElementGraphics
SET SrcDir=%SrcRoot%DgnPlatform\DgnCore\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%DgnPlatform\PrivateApi\DgnPlatformInternal\DgnCore\
SET OutFile=%OutDir%%BaseName%.fb.h

SET CompileCmd=%SrcRoot%libsrc\flatbuffers\bin\beflatc.exe -c

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
    GOTO:EOF
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
