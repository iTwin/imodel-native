@ECHO OFF
rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See LICENSE.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

: KEEP THIS FILE IN SYNC ACROSS ALL NATIVE GEOMLIBS REPOS:
: * PPBase\Geomlibs\serialization\src\FlatBuffer\CompileFbs.bat
: * imodel-native\iModelCore\GeomLibs\serialization\src\FlatBuffer\CompileFbs.bat
: * imodel02\iModelCore\GeomLibs\serialization\src\FlatBuffer\CompileFbs.bat

: NOTES:
: * This Windows script compiles the flatbuffer geometry schema allcg.fbs into flatbuffer accessors for Bentley's
:   native, iTwin, and .NET core geometry libraries.
: * Run this script whenever data is added to a geometry type that must be persisted, then manually copy its outputs
:   to the respective repos:
:   * C++ output is %OutDir%allcg_generated.h, and belongs in:
:     * PPBase\Geomlibs\serialization\src\FlatBuffer\
:     * imodel02\iModelCore\GeomLibs\serialization\src\FlatBuffer\
:     * imodel-native\iModelCore\GeomLibs\serialization\src\FlatBuffer\  <--automatically copied by this script
:   * TypeScript output is %OutDir%BGFBAccessors.ts, and belongs in:
:     * itwinjs\core\geometry\src\serialization\
:   * .NET output is %OutDir%Bentley\GeometryNET\FB\*.cs, and belongs in:
:     * PPBase\BentleyGeometryNet\src\FlatBuffers\gensrc\
: * Do not commit %OutDir% and %TempDir%. After copying outputs, manually delete these directories. %TempDir% is
:   automatically deleted by this script on successful completion.
: * The Google flatbuffers github repo has diverged too much to efficiently port Bentley changes and/or analyze the
:   generated accessor deltas. Lacking a compelling reason to upgrade, we continue to employ the original version
:   of the flatbuffers compiler (with Bentley additions) that we first used to compile the TypeScript accessors:
:   https://github.com/google/flatbuffers/releases/tag/v1.12.0 .

SET BeFlatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\beflatc.exe
SET GemaExe=%OutRoot%Tools\bsitools\gema.exe

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
%BeFlatcExe% -c -o %TempDir% %TempFile% || (
    @ECHO Native compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.h
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

: modernize copyright
SET OutFile=%OutDir%%BaseName%_generated.h
%GemaExe% -f %SrcDir%fixupNative.g %GeneratedFile% > %OutFile%

@ECHO Done Generating %OutFile%
COPY %OutFile% %SrcDir%

: Typescript accessors: core\geometry\src\serialization\BGFBAccessors.ts
@ECHO Compiling Typescript accessors...
%BeFlatcExe% --ts -o %TempDir% %SrcFile% || (
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
%BeFlatcExe% -n -o %OutDir% %TempFile% || (
    @ECHO .NET compile failed
    goto done
    )
@ECHO Done Generating %OutDir%Bentley\GeometryNET\FB\*.cs

@RD /Q %TempDir%
:done