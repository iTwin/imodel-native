echo This file has been replaced by "linknativeplatform.bat". You will need to set itwinjsDir to use it.

@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

set tempDir=%imodeljsDir%

REM Install local build of the platform-specific native platform packages for use by examples and tests
if not exist %tempDir%\core\backend goto :badJsdir
if not exist %tempDir%\full-stack-tests\backend goto :badJsdir

if defined coreDestDir if defined testDestDir goto :doCopy
set coreDestDir=%tempDir%\core\backend\node_modules\@bentley\imodeljs-native
set testDestDir=%tempDir%\full-stack-tests\backend\node_modules\@bentley\imodeljs-native

:doCopy
echo on
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native %coreDestDir%
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-win32-x64 %coreDestDir%\imodeljs-win32-x64
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native %testDestDir%
xcopy /QYIS %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-win32-x64 %testDestDir%\imodeljs-win32-x64
@echo off

REM Copy the LICENSE.md mastered in the iModelJsNodeAddon repository into the imodeljs repository so that we can link to it when imodeljs is posted to GitHub
copy %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native\LICENSE.md %tempDir%\core\backend\src\imodeljs-native-LICENSE.md

goto :xit

:badJsDir
echo Set the environment variable imodeljsDir that points to an imodeljs directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
