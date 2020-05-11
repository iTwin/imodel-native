@echo off

rem /*--------------------------------------------------------------------------------------+
rem |
rem |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
rem |  See COPYRIGHT.md in the repository root for full copyright notice.
rem |
rem +--------------------------------------------------------------------------------------*/

rem SQLite uses the Fossil source code management system.
rem You can track Blockcachevfs development at https://sqlite.org/blockcachevfs/timeline

rem /*--------------------------------------------------------------------------------------+
rem Prerequisites
rem +--------------------------------------------------------------------------------------*/
rem * Fossil source code management
rem     - Install from http://www.fossil-scm.org and ensure fossil.exe is in your environment path
rem * BlockCacheVFS source code
rem     - pull from sqlite.org (requires fossil). In shell:
rem        e:\] mkdir blockcachevfs
rem        e:\] cd blockcachevfs
rem        e:\blockcachevfs\] fossil clone http://www.sqlite.org/blockcachevfs blockcachevfs.fossil
rem        e:\blockcachevfs\] fossil open blockcachevfs.fossil
rem * set BcvfsSrcRoot environment var to point to the location of the blockcachevfs source code

@IF "%BcvfsSrcRoot%." == "." goto BcvfsSrcRootNotDefined

rem Check out source
pushd %BcvfsSrcRoot%
fossil update trunk

rem Copy to bentley source tree
set BeSqlOutRoot=%SrcRoot%imodel02\iModelCore\BeSQLite\SQLite\

copy src\bcv_socket.h %BeSqlOutRoot%
copy src\blockcachevfs.c %BeSqlOutRoot%
copy src\blockcachevfs.h %BeSqlOutRoot%
copy src\blockcachevfsd.c %BeSqlOutRoot%
copy src\simplexml.c %BeSqlOutRoot%
copy src\simplexml.h %BeSqlOutRoot%

popd

goto END

:BcvfsSrcRootNotDefined
echo You must define BcvfsSrcRoot to point to your blockcachevfs source directory

:END

