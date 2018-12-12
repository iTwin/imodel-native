set PTIO=%POINTOOLS%\PointoolsIO\workingbuild\bin
set PTV=%POINTOOLSVORTEX%\bin\vc8
set DEST=%PTMS%

copy "%PTIO%\PodFormatsd.dll" "%DEST%\PodFormats.dll"
copy "%PTIO%\PodWriterd.dll" "%DEST%\PodWriter.dll"
copy "%PTIO%\..\..\deps\x86\*.dll" "%DEST%\"
copy "%PTV%\PointoolsVortexAPId.dll" "%DEST%\PointoolsVortexAPI.dll"

