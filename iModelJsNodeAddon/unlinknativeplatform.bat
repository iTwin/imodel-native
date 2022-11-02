@echo off
SETLOCAL

if not defined OutRoot goto :badOutRoot

REM unlink local build from itwinjs builds

if not exist %itwinjsDir%\core\backend goto :badJsdir

:doUnlink

rmdir /s/q %itwinjsDir%\core\backend\node_modules\@bentley\imodeljs-native

echo Development build unlinked from %itwinjsDir%.

goto :xit

:badJsDir
echo Set the environment variable itwinjsDir that points to an iTwin.js directory.
goto :xit

:badOutRoot
echo The OutRoot environment variable is not set.

:xit
