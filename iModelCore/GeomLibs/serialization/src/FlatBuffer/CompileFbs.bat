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
: * This Windows script manually to compile the flatbuffer geometry schema allcg.fbs into flatbuffer accessors for
:   native, iTwin, and .NET geometry libraries.
: * Run this script whenever data is added to a geometry type that must be persisted, then ensure outputs
:   are copied to the respective repos:
:   * C++ output is %OutDir%allcg_generated.h, and belongs in:
:     * PPBase\Geomlibs\serialization\src\FlatBuffer\
:     * imodel02\iModelCore\GeomLibs\serialization\src\FlatBuffer\
:     * imodel-native\iModelCore\GeomLibs\serialization\src\FlatBuffer\
:   * Typescript output is %OutDir%BGFBAccessors.ts, and belongs in itwinjs\core\geometry\src\serialization\
:   * .NET output is %OutDir%Bentley\GeometryNET\FB\*.cs, and belongs in PPBase\BentleyGeometryNet\src\FlatBuffers\gensrc\
: * The Google flatbuffers repo has diverged too much to efficiently port our changes and/or analyze modern output
:   differences in the generated geometry FB accessors for TypeScript and .NET. Therefore in this script, we continue
:   to employ the same compilers first used to generate the geometry FB accessors for these languages.

: beflatc is the latest version built in libsrc with Bentley changes, and used to generate C++ accessors.
SET CFlatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\beflatc.exe

: flatc v1.12.0 is an early Google release first used to generate Typescript accessors.
SET TSFlatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\flatc.1.12.0.exe

: beflatc v1.0.0 is the original Bentley version first used to generate .NET accessors.
SET NETFlatcExe=%SrcRoot%imodel-native\iModelCore\libsrc\flatbuffers\bin\beflatc.1.0.0.exe

: gema is open-source pattern-based text processor developed by David N. Gray and used in several build scripts.
SET GemaExe=%SrcRoot%toolcache\bsitools_x64.1.0.0-6\gema.exe

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
%CFlatcExe% -c -o %TempDir% %TempFile% || (
    @ECHO Native compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.h
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

: TODO: this may not be necessary if beflatc.exe has been updated to insert modern copyright
SET OutFile=%OutDir%%BaseName%_generated.h
%GemaExe% -f %SrcDir%fixupNative.g %GeneratedFile% > %OutFile%

@ECHO Done Generating %OutFile%
COPY %OutFile% %SrcDir%

: Typescript accessors: core\geometry\src\serialization\BGFBAccessors.ts
@ECHO Compiling Typescript accessors...
%TSFlatcExe% --ts -o %TempDir% %SrcFile% || (
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
%NETFlatcExe% -n -o %OutDir% %TempFile% || (
    @ECHO .NET compile failed
    goto done
    )
@ECHO Done Generating %OutDir%Bentley\GeometryNET\FB\*.cs

@RD /Q %TempDir%
:done