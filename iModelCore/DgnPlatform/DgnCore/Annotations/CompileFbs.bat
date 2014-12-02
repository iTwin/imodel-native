@ECHO OFF

rem ------------------------------------------------------------------------------------
rem      $Source: DgnCore/Annotations/CompileFbs.bat $
rem   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
rem ------------------------------------------------------------------------------------

SET BaseName=Annotations
SET SrcDir=%SrcRoot%DgnPlatform\DgnCore\Annotations\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET GeneratedDir=%SrcDir%
SET GeneratedFileName=%BaseName%_generated.h
SET GeneratedFile=%GeneratedDir%%GeneratedFileName%
SET OutDir=%SrcRoot%DgnPlatform\PrivateApi\DgnPlatformInternal\DgnCore\Annotations\
SET OutFile=%OutDir%%BaseName%.fb.h

SET CompileCmd=%SrcRoot%libsrc\flatbuffers\bin\beflatc.exe -c

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

MOVE %GeneratedFile% %OutFile% > NUL || (
    ECHO Could not move header '%GeneratedFile%' -> '%OutFile%'
    EXIT 1
    )

ECHO.    Done.
rem ***********************************************************************************************
rem ***********************************************************************************************
ECHO Successfully created:
ECHO.    '%OutFile%'
