set PTIO=%POINTOOLS%\PointoolsIO\workingbuild\bin
set PTV=%POINTOOLSVORTEX%\bin
set DEST=%programfiles(x86)%\Bentley 08.11.09.184\MicroStation\mdlsys\asneeded\pointools

copy "%PTIO%\*.dll" "%DEST%\"
copy "%PTV%\PointoolsVortexAPI.dll" "%DEST%\PointoolsVortexAPI.dll"
copy "%PTV%\PointoolsVortexAPId.dll" "%DEST%\PointoolsVortexAPI.dll"

