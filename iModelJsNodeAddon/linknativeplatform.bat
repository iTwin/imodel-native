@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

REM link local build to itwinjs builds for testing

if not exist %itwinjsDir%\core\backend goto :badJsdir

if defined coreDestDir goto :doCopy
set coreDestDir=%itwinjsDir%\core\backend\node_modules\@bentley\imodeljs-native

:doCopy

REM We must make the "lib" directory in our source tree look like the post-install step has been run on it. This makes
REM a symbolic link from the output tree into the source tree, which is wierd but that's because we build the typescript
REM directly into our source tree. Oh well.

set libDir=%OutRoot%Winx64\BuildContexts\iModelJsNodeAddon\Delivery\lib\

if exist %libDir%imodeljs-win32-x64 rmdir /s /q %libDir%imodeljs-win32-x64
mklink /j %libDir%imodeljs-win32-x64 %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-win32-x64 >nul

copy %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native\package.json %libDir% >nul

if not exist %libDir%\devbuild.json echo {"dev-build": true} > %libDir%\devbuild.json

if exist %coreDestDir% rmdir /s /q %coreDestDir%
mklink /j %coreDestDir% %libDir% >nul

echo Local build in %libDir% is linked now to %itwinjsDir%. Use "unlinknativebuild" to stop.

goto :xit

:badJsDir
echo Set the environment variable itwinjsDir that points to an iTwin.js directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
