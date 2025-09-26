@echo on

rem /*--------------------------------------------------------------------------------------+
rem |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem |  See LICENSE.md in the repository root for full copyright notice.
rem +--------------------------------------------------------------------------------------*/

rem This script will update the SQLite source from a fossil branch, and then create the amalgamation `sqlite.c` source file.
rem @note that it requires cygwin be installed

set sqlite_tag=itwin-sqlite-v3.50.4-r0
set imodel_native_sqlite=%SrcRoot%imodel-native\iModelCore\BeSQLite\SQLite\
set sqlite_root=%appdata%\itwin-sqlite
set make_target=sqlite3.c

rem update tags
git fetch --tags
rem create folder in appdata if it does not already exist.
if not exist %sqlite_root% md %sqlite_root%

rem save current folder
pushd .
cd /D %sqlite_root%

rem download or update sqlite source to appdata
if not exist %sqlite_root%\configure (
    git clone https://github.com/iTwin/sqlite.git .
)

git checkout master -f
git pull

if %errorlevel% neq 0 (
  echo "*** Failed to pull from iTwin/sqlite ***""
  popd
  exit /b %errorlevel%
)

git switch %sqlite_tag% --detach

if %errorlevel% neq 0 (
  echo "*** Failed to switch source to tag: %sqlite_tag% ***""
  popd
  exit /b %errorlevel%
)

nmake /f Makefile.msc clean sqlite3.c

copy %make_target% %imodel_native_sqlite%
copy sqlite3.h %imodel_native_sqlite%
copy shell.c %imodel_native_sqlite%

popd

echo "*** Completed successfully ***"

:end
