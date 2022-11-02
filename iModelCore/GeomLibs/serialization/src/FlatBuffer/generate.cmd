@REM Copyright (c) Bentley Systems, Incorporated. All rights reserved.
@REM See LICENSE.md in the repository root for full copyright notice.

@echo Generating primary h file ...
@echo .
@echo .
rem %srcRoot%libsrc\flatbuffers\bin\beflatc.exe -c allcg.flatbuf
rem c:\bin\flatcv1.12.0.exe -c allcg.fbs
rem c:\bin\flatc17.exe -c allcg.fbs
%srcRoot%iModel02\iModelCore\libsrc\flatbuffers\bin\beflatc.exe -c allcg.fbs
@echo .
@echo .

touch FixedStructs.cpp


