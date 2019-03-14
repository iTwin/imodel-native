@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

set tempDir=%imodeljsDir%

REM Install local build of the platform-specific native platform packages for use by examples and tests
if not exist %tempDir%\core\backend goto :badJsdir

set destDir=%tempDir%\core\backend\node_modules\@bentley\imodeljs-native
echo on
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native %destDir%
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-win32-x64 %destDir%\imodeljs-win32-x64
@echo off

REM Create a semaphore file that identifies that this is a dev build, and helps bypass version checks
set libDir=%tempDir%\core\backend\lib
if not exist %libDir% mkdir %libDir%
echo This is a Development build, and this semaphore helps bypass native version checks > %libDir%\DevBuild.txt

goto :xit

:badJsDir
echo Set the environment variable imodeljsDir that points to an imodeljs directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
