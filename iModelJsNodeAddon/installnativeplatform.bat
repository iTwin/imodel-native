echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

if [%1] == [] (
  if not defined imodeljsDir (
    echo testing
  )
  set tempDir=%imodeljsDir%
) else (
  set tempDir=%1
)

if not exist %tempDir% goto :badJsDir

REM Install local build of the platform-specific native platform packages for use by examples and tests
if not exist %tempDir%\common goto :badJsdir

xcopy /Q /Y /I %OutRoot%Winx64\packages\imodeljs-native-platform-api %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api
xcopy /Q /Y /I /S %OutRoot%Winx64\packages\imodeljs-n_8-win32-x64    %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api\lib\@bentley\imodeljs-n_8-win32-x64
xcopy /Q /Y /I /S %OutRoot%Winx64\packages\imodeljs-e_2-win32-x64    %tempDir%\common\temp\node_modules\@bentley\imodeljs-native-platform-api\lib\@bentley\imodeljs-e_2-win32-x64

REM Create a semaphore file that identifies that this is a dev build, and helps bypass version checks
set libDir=%tempDir%\core\backend\lib
if not exist %libDir% mkdir %libDir%
echo This is a Development build, and this semaphore helps bypass native version checks > %libDir%\DevBuild.txt

goto :xit

:badJsDir
echo Set the environment variable imodeljsDir, or pass an argument, that points to an imodeljs directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
