set PTIO=%POINTOOLS%\PointoolsIO\workingbuild\bin
set PTV=%POINTOOLSVORTEX%\bin\vc8
set DEST=%PTMS%

copy "%PTIO%\PodFormats.dll" "%DEST%\"
copy "%PTIO%\PodWriter.dll" "%DEST%\"
copy "%PTIO%\..\..\deps\x86\*.dll" "%DEST%\"
copy "%PTV%\PointoolsVortexAPI.dll" "%DEST%\PointoolsVortexAPI.dll"

