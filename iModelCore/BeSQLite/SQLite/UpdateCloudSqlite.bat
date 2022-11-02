@echo off

rem /*--------------------------------------------------------------------------------------+
rem |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem |  See LICENSE.md in the repository root for full copyright notice.
rem +--------------------------------------------------------------------------------------*/

rem This script will update the SQLite Cloud source from a fossil branch.

set besqlite_root=%SrcRoot%imodel02\iModelCore\BeSQLite\SQLite\
set sqlite_root=%appdata%\sqlite-cloud-src
set fossil_db=sqlite.fossil


@if "%SqliteCloudBranch%." == "." set SqliteCloudBranch=trunk

rem create folder in appdata if it does not already exist.
if not exist %sqlite_root% md %sqlite_root%

rem save current folder
pushd .
cd /D %sqlite_root%

rem download or update sqlite source to appdata
if exist %sqlite_root%\%fossil_db% (
    fossil update %SqliteCloudBranch%
) else (
    fossil clone https://sqlite.org/cloudsqlite %fossil_db%
    fossil open %fossil_db%
    fossil update %SqliteCloudBranch%
)
if %errorlevel% neq 0 (
  echo "*** Unable to clone or update sqlite source code ***"
  popd
  exit /b %errorlevel%
)

copy src\bcv_int.h %besqlite_root%
copy src\bcvutil.h %besqlite_root%
copy src\bcvmodule.h %besqlite_root%
copy src\bcvmodule.c %besqlite_root%
copy src\bcvutil.c %besqlite_root%
copy src\blockcachevfs.c %besqlite_root%
copy src\blockcachevfs.h %besqlite_root%
copy src\blockcachevfsd.c %besqlite_root%
copy src\simplexml.c %besqlite_root%
copy src\simplexml.h %besqlite_root%

popd

echo "*** Completed successfully ***"

:end