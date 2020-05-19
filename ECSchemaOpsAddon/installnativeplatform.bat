@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

set tempDir=%iModelBankDir%

if not exist %tempDir%\server goto :badJsdir

set destDir=%tempDir%\server\node_modules\@bentley\imodel-bank
echo on
xcopy /QYIS %OutRoot%\Winx64\ecschemaopsaddon_pkgs\ecschema-ops           %destDir%
xcopy /QYIS %OutRoot%\Winx64\ecschemaopsaddon_pkgs\ecschema-ops-win32-x64 %destDir%\ecschema-ops-win32-x64
@echo off

REM Create a semaphore file that identifies that this is a dev build, and helps bypass version checks
set libDir=%tempDir%\server\lib
if not exist %libDir% mkdir %libDir%
echo This is a Development build, and this semaphore helps bypass native version checks > %libDir%\DevBuild.txt

goto :xit

:badJsDir
echo Set the environment variable iModelBankDir that points to the top-level directory of an imodel-bank repository.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
