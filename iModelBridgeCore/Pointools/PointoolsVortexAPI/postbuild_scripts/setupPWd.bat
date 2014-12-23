net stop PWAppSrv

set PTIO=%POINTOOLS%\PointoolsIO\workingbuild\bin
set PTV=%POINTOOLSVORTEX%\bin\vc8
set DEST=C:\Program Files\Bentley\ProjectWise\Bin\PointCloud

@REM copy "%PTIO%\*.dll" "%DEST%\"
xcopy /Y "%PTV%\PointoolsVortexAPI64d.dll" "%DEST%\PointoolsVortexAPI64.dll"

xcopy /Y %POINTOOLSVORTEX%\obj\debug-vc8-x64\PointoolsAPI.pdb "%DEST%\"

net start PWAppSrv
