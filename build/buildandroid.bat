setlocal

set _SrcRoot=%~dp0..\..\
set BMAKE_OPT=-I%_SrcRoot%\imodel-native\build\PublicSDK
set ToolCache=%_SrcRoot%\imodel-native\build\toolcache\
set NDEBUG=1
set | findstr /i debug
py -3 %_SrcRoot%\imodel-native\build\BentleyBuild\BentleyBuild.py -s%_SrcRoot%\imodel-native\build\strategies\iModelJsNodeAddonOpen.BuildStrategy.xml -aAndroidArm64 --srcroot=%_SrcRoot%\ --outputroot=%_SrcRoot%\..\out build %*
endlocal
