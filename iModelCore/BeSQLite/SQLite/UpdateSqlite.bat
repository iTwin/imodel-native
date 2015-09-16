@echo off

rem /*--------------------------------------------------------------------------------------+
rem |
rem |     $Source: SQLite/UpdateSqlite.bat $
rem |
rem |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
rem |
rem +--------------------------------------------------------------------------------------*/

rem SQLite uses the Fossil source code management system.
rem Bentley pulls SQLite from the "sessions" branch, to which the SQLite developers regularly merge.
rem You can track SQLite development at http://www.sqlite.org/src/timeline

rem Bentley uses bentleybuild to compile SQLite, but the source code must be preprocessed first.
rem The hundred or so .c files are consolidated into one, then split into smaller chunks (due to a limitation of Microsoft's compiler).
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
fossil update sessions

rem Set up build directory
rmdir /s/q bld
mkdir bld
cd bld
bash ..\configure

rem Process the source
make sqlite3.c
make sqlite3-all.c

rem Copy to bentley source tree
set BeSqlOutRoot=%SrcRoot%BeSQLite\SQLite\
copy sqlite3-1.c            %BeSqlOutRoot%
copy sqlite3-2.c            %BeSqlOutRoot%
copy sqlite3-3.c            %BeSqlOutRoot%
copy sqlite3-4.c            %BeSqlOutRoot%
copy sqlite3-5.c            %BeSqlOutRoot%
copy sqlite3-6.c            %BeSqlOutRoot%
copy sqlite3.c              %BeSqlOutRoot%sqlite3.amalgam.c
copy shell.c                %BeSqlOutRoot%
copy ..\ext\misc\json1.c    %BeSqlOutRoot%
copy sqlite3.h              %BeSqlOutRoot%

popd

goto END

:SqliteSrcRootNotDefined
echo You must define SqliteSrcRoot to point to your sqlite source directory

:END

