@ECHO OFF
rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See LICENSE.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

: Geometry developers run this locally (Windows) to compile the flatbuffer
: geometry schema allcg.fbs into flatbuffer accessors for native, iTwin, and
: .NET geometry libraries. This is necessary whenever data is added to a
: geometry type that must be persisted across all 3 geometry repos.

: flatc version 1.0.0 is compiled in libsrc with Bentley additions
SET BeflatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\beflatc.1.0.0.exe

: flatc version 1.12.0 adds Typescript generation
SET FlatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\flatc.1.12.0.exe

: gema is open-source pattern-based text processor developed by David N. Gray
SET GemaExe=%SrcRoot%toolcache\bsitools_x64.1.0.0-6\gema.exe

: NOTES:
: * Native output is generated in place.
: * iTwin/.NET outputs in %OutDir% must be hand-copied to their respective repo.
: * %OutDir% and %TempDir% will be created locally and should not be committed.
: * The Google flatbuffers repo has diverged too much to efficiently port our
:   changes and understand output differences, so we use legacy versions here.

SET BaseName=allcg
SET SrcDir=.\
SET SrcFile=%SrcDir%%BaseName%.fbs
SET OutDir=%SrcDir%out\
SET TempDir=%SrcDir%temp\
SET TempFile=%TempDir%%BaseName%.tmp

IF NOT EXIST %OutDir% MKDIR %OutDir% || (
    @ECHO Could not create '%OutDir%'
    goto done
    )

IF NOT EXIST %TempDir% MKDIR %TempDir% || (
    @ECHO Could not create '%TempDir%'
    goto done
    )

: Native accessors: allcg_generated.h
@ECHO namespace Bentley.Geometry.FB; > %TempFile%
TYPE %SrcFile% >> %TempFile%
@ECHO Compiling native accessors...
%BeflatcExe% -c -o %TempDir% %TempFile% || (
    @ECHO Native compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.h
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

SET OutFile=%SrcDir%%BaseName%_generated.h
%GemaExe% -f %SrcDir%fixupNative.g %GeneratedFile% > %OutFile%
@ECHO Done Generating %OutFile%

: Typescript accessors: core\geometry\src\serialization\BGFBAccessors.ts
@ECHO Compiling Typescript accessors...
%FlatcExe% --ts -o %TempDir% %SrcFile% || (
    @ECHO Typescript compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.ts
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

SET OutFile=%OutDir%BGFBAccessors.ts
SET TypescriptDir=%SrcDir%typescript\
%GemaExe% -f %TypescriptDir%fixupTypescript.g %GeneratedFile% > %OutFile%
@ECHO Done Generating %OutFile%

: .NET accessors: PPBase\BentleyGeometryNet\src\FlatBuffers\gensrc\*.cs
@ECHO namespace Bentley.GeometryNET.FB; > %TempFile%
TYPE %SrcFile% >> %TempFile%
@ECHO Compiling .NET accessors...
%BeflatcExe% -n -o %OutDir% %TempFile% || (
    @ECHO .NET compile failed
    goto done
    )
@ECHO Done Generating %OutDir%Bentley\GeometryNET\FB\*.cs

:done