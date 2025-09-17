@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

REM link native build to ecsql fuzzer build for testing code coverage

if not defined fuzzerDir goto :badFuzzerDir

if not defined fuzzerDestDir set fuzzerDestDir=%fuzzerDir%\node_modules\@bentley\imodeljs-native

:doCopy

REM We must make the "lib" directory in our source tree look like the post-install step has been run on it. This makes
REM a symbolic link from the output tree into the source tree, which is wierd but that's because we build the typescript
REM directly into our source tree. Oh well.

set libDir=%OutRoot%Winx64\BuildContexts\iModelJsNodeAddon\Delivery\lib\

if exist %libDir%imodeljs-win32-x64 rmdir /s /q %libDir%imodeljs-win32-x64
mklink /j %libDir%imodeljs-win32-x64 %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-win32-x64 >nul

copy %OutRoot%Winx64\imodeljsnodeaddon_pkgs\imodeljs-native\package.json %libDir% >nul

if not exist %libDir%\devbuild.json echo {"dev-build": true} > %libDir%\devbuild.json

echo Local build in %libDir% is linked now to %fuzzerDir%.

if exist %fuzzerDestDir% rmdir /s /q %fuzzerDestDir%
mklink /j %fuzzerDestDir% %libDir% >nul

goto :xit

:badFuzzerDir
echo Set the environment variable fuzzerDir that points to the fuzzing directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
