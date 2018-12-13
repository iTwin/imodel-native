echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

set tempDir=%imodeljsDir%

if not exist %tempDir% goto :badJsDir

REM Install local build of the platform-specific native platform packages for use by examples and tests
if not exist %tempDir%\common goto :badJsdir

for /f "delims=v." %%v in ('node --version') do set NODE_MAJOR_VERSION=%%v

echo Node major version is %NODE_MAJOR_VERSION%

if %NODE_MAJOR_VERSION%==10 set ELECTRON_MAJOR_VERSION=3

echo Electron major version is %ELECTRON_MAJOR_VERSION%

xcopy /Q /Y /I %OutRoot%Winx64\packages\imodeljs-native-platform-api %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api
xcopy /Q /Y /I /S %OutRoot%Winx64\packages\imodeljs-n_%NODE_MAJOR_VERSION%-win32-x64      %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api\lib\@bentley\imodeljs-n_%NODE_MAJOR_VERSION%-win32-x64
xcopy /Q /Y /I /S %OutRoot%Winx64\packages\imodeljs-e_%ELECTRON_MAJOR_VERSION%-win32-x64  %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api\lib\@bentley\imodeljs-e_%ELECTRON_MAJOR_VERSION%-win32-x64

REM Copy the LICENSE.md mastered in the iModelJsNodeAddon repository into the imodeljs repository so that we can link to it when imodeljs is posted to GitHub
copy %OutRoot%Winx64\packages\imodeljs-native-platform-api\LICENSE.md %tempDir%\core\backend\src\imodeljs-native-LICENSE.md

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
