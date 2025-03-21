@ECHO OFF
rem ------------------------------------------------------------------------------------
rem   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem   See LICENSE.md in the repository root for full copyright notice.
rem ------------------------------------------------------------------------------------

: NOTES:
: * This Windows script compiles the flatbuffers geometry schema allcg.fbs into flatbuffers accessors for Bentley's
:   native, iTwin, and .NET core geometry libraries.
: * Changes to allcg.fbs and this file must be reflected in all 3 native geomlibs repos: PPBase, imodel-native, imodel02.
: * Whenever allcg.fbs changes, or whenever the flatbuffers distribution is updated, run this script in all 3
:   locations, and follow its directions.
: * The allcg.fbs schema should only be changed by appending new persistent data to a geometry type.
: * Do not commit %TempDir% and %OutDir%:
:   * %TempDir% is automatically deleted on successful completion of this script.
:   * %OutDir% will contain generated file(s) to be manually copied elsewhere.
: * The Google flatbuffers GitHub repo has diverged too much to efficiently port Bentley changes and/or analyze the
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

: Native accessors
@ECHO Compiling native accessors...
%BeFlatcExe% -c -o %TempDir% %SrcFile% || (
    @ECHO Native compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.h
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

SET OutFile=%SrcDir%%BaseName%_generated.h
%GemaExe% -f %SrcDir%fixup\fixupNative.g %GeneratedFile% > %OutFile%
@ECHO Done Generating %OutFile%

: Typescript accessors
@ECHO Compiling Typescript accessors...
%BeFlatcExe% --ts -o %TempDir% %SrcFile% || (
    @ECHO Typescript compile failed
    goto done
    )
SET GeneratedFile=%TempDir%%BaseName%_generated.ts
IF NOT EXIST %GeneratedFile% @ECHO Failed to generate '%GeneratedFile%' && goto done

SET OutFile=%OutDir%BGFBAccessors.ts
%GemaExe% -f %SrcDir%fixup\fixupTypescript.g %GeneratedFile% > %OutFile%
@ECHO Done Generating %OutFile%

@ECHO FlatBuffer compilation is complete.
@ECHO -------------------------------------------
@ECHO Further Instructions (if not already done):
@ECHO -------------------------------------------
@ECHO 1. Copy %OutFile% to itwinjs\core\geometry\src\serialization, then delete %OutDir%
@ECHO 2. Copy %SrcFile% to PPBase\Geomlibs\serialization\src\FlatBuffer
@ECHO 3. Run PPBase\Geomlibs\serialization\src\FlatBuffer\CompileFbs.bat
@ECHO 4. Copy %SrcFile% to imodel02\iModelCore\GeomLibs\serialization\src\FlatBuffer
@ECHO 5. Run imodel02\iModelCore\GeomLibs\serialization\src\FlatBuffer\CompileFbs.bat
@ECHO ---------------------

@RD /Q/S %TempDir%
:done