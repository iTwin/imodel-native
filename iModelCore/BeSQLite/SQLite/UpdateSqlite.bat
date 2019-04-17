@echo off

rem /*--------------------------------------------------------------------------------------+
rem |
rem |
rem |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem |
rem +--------------------------------------------------------------------------------------*/

rem SQLite uses the Fossil source code management system.
rem You can track SQLite development at http://www.sqlite.org/src/timeline

rem Bentley uses bentleybuild to compile SQLite, but the source code must be preprocessed first.
rem The hundred or so .c files are consolidated into one.
rem This is accomplished using TCL scripts.

rem /*--------------------------------------------------------------------------------------+
rem Prerequisites
rem +--------------------------------------------------------------------------------------*/
rem * Fossil source code management
rem     - Install from http://www.fossil-scm.org and ensure fossil.exe is in your environment path
rem * Bash, make, and TCL
rem     - Easiest way on Windows is to install/update Cygwin and ensure these utilities are selected for installation
rem * SQLite source code
rem     - pull from sqlite.org (requires fossil). In shell:
rem        e:\] mkdir sqlite
rem        e:\] cd sqlite
rem        e:\sqlite\] fossil clone http://www.sqlite.org/cgi/src sqlite.fossil
rem        e:\sqlite\] fossil open sqlite.fossil
rem * set SqliteSrcRoot environment var to point to the location of the sqlite source code

@IF "%SqliteSrcRoot%." == "." goto SqliteSrcRootNotDefined

rem Check out source
pushd %SqliteSrcRoot%
REM branch-3.22
fossil update trunk

rem Set up build directory
rmdir /s/q bld
mkdir bld
cd bld
bash ..\configure

rem Process the source
make sqlite3.c

rem Copy to bentley source tree
set BeSqlOutRoot=%SrcRoot%BeSQLite\SQLite\
copy sqlite3.c %BeSqlOutRoot%
copy shell.c %BeSqlOutRoot%
copy sqlite3.h %BeSqlOutRoot%

popd

goto END

:SqliteSrcRootNotDefined
echo You must define SqliteSrcRoot to point to your sqlite source directory

:END

