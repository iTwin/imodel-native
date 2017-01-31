@echo off
rem Usage
rem    build <FileNameStem>
rem
rem Invoke cl.exe for a single-step compile and link of FileNameStem.cpp
rem
rem The SimpleCompile.opts file will need to be localized to reference the following:
rem 1) The bentley "sharedinc" directory containing bentley.h and friends.
rem 2) In the bsibasegeom delivery, the include directories basegeom\include and vu\include
rem 3) In the bsibasegeom delivery, the library directories for bsibasegeom.lib and bsivu.lib
rem
rem At run time, the PATH must have bsibasegeom.dll and bsivu.dll
rem

if .%1 EQU . goto AllFiles

set SOURCEFILENAME=%1.cpp
set EXEFILENAME=%1.exe

if EXIST %SOURCEFILENAME% goto SingleFile

:Usage
    echo To build one filename.cpp:  build <filename>
    echo To build all files:         build
    goto Done


:SingleFile
    cl @SimpleCompile.opts  %SOURCEFILENAME% -Fe%EXEFILENAME%
    goto Done

:AllFiles
    for %%f in (*.cpp) do cl @SimpleCompile.opts %%~nf.cpp -Fe%%~nf.exe
    goto Done

:Done