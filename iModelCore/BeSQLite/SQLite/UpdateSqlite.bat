@echo off

rem /*--------------------------------------------------------------------------------------+
rem |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem |  See LICENSE.md in the repository root for full copyright notice.
rem +--------------------------------------------------------------------------------------*/

rem This script will update the SQLite source from a fossil branch, and then create the amalgamation `sqlite.c` source file.
rem @note that it requires cygwin be installed

set besqlite_root=%SrcRoot%imodel02\iModelCore\BeSQLite\SQLite\
set sqlite_root=%appdata%\sqlite-src
set fossil_db=sqlite.fossil
set build_dir=out
set target=sqlite3.c

@if "%SqliteBranch%." == "." set SqliteBranch=trunk

rem create folder in appdata if it does not already exist.
if not exist %sqlite_root% md %sqlite_root%

rem save current folder
pushd .
cd /D %sqlite_root%

rem download or update sqlite source to appdata
if exist %sqlite_root%\%fossil_db% (
    fossil update %SqliteBranch%
) else (
    fossil clone https://www.sqlite.org/src %fossil_db%
    fossil open %fossil_db%
    fossil update %SqliteBranch%
)
if %errorlevel% neq 0 (
  echo "*** Unable to clone or update sqlite source code ***"
  popd
  exit /b %errorlevel%
)

rem configure and build sqlite
if exist %build_dir% rmdir /s/q %build_dir%
mkdir %build_dir%
cd %build_dir%
sh ..\configure
if %errorlevel% neq 0 (
  rmdir /s/q %build_dir%
  echo "*** Failed to configure sqlite source code ***"
  popd
  exit /b %errorlevel%
)

rem Build the target
make %target%
if %errorlevel% neq 0 (
  rmdir /s/q %build_dir%
  echo "*** Failed to build sqlite source code ***"
  popd
  exit /b %errorlevel%
)
rem Copy to bentley source tree

copy %target% %besqlite_root%
copy sqlite3.h %besqlite_root%
copy shell.c %besqlite_root%

popd

echo "*** Completed successfully ***"

:end